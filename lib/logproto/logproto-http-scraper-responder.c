/*
 * Copyright (c) 2025 One Identity
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */
#include "logproto/logproto-http-scraper-responder.h"
#include "stats/stats-prometheus.h"
#include "stats/stats-query-commands.h"
#include "timeutils/cache.h"
#include "messages.h"

#include <iv.h>
#include <glib.h>

static LogProtoHTTPScraperResponder *single_instance;
static time_t last_scrape_request_time;

static inline GMutex *
_mutex(void)
{
  static GMutex *mutex = NULL;

  if (g_once_init_enter(&mutex))
    {
      GMutex *new_mutex = g_new(GMutex, 1);
      g_mutex_init(new_mutex);
      g_once_init_leave(&mutex, new_mutex);
    }
  return mutex;
}

static void
_generate_batched_response(const gchar *record, gpointer user_data)
{
  gpointer *args = (gpointer *) user_data;
  //LogProtoHTTPScraperResponder *self = (LogProtoHTTPScraperResponder *)args[0];
  GString **batch = (GString **) args[1];

  g_string_append_printf(*batch, "%s", record);
}

static GString *
_compose_prometheus_response_body(LogProtoHTTPServer *s)
{
  LogProtoHTTPScraperResponder *self = (LogProtoHTTPScraperResponder *)s;

  GString *stats = NULL;
  gboolean cancelled = FALSE;

  if (self->options->stat_type == STT_STAT)
    {
      stats = g_string_new(NULL);
      gpointer args[] = {self, &stats};
      gboolean with_legacy = TRUE;
      msg_trace("Generating prometheus stat");
      stats_generate_prometheus(_generate_batched_response, args, with_legacy, &cancelled);
    }
  else
    {
      char *query_str = self->options->stat_query && self->options->stat_query[0] ? self->options->stat_query : "*";
      GString *full_query_str = g_string_new(NULL);
      g_string_append_printf(full_query_str, "QUERY GET prometheus %s", query_str);

      msg_trace("Running prometheus query", evt_tag_str("stat-query", query_str));
      stats = stats_execute_query_command(full_query_str->str, self, &cancelled);
      g_string_free(full_query_str, TRUE);
    }
  return stats;
}

static GString *
_compose_response_body(LogProtoHTTPServer *s)
{
  LogProtoHTTPScraperResponder *self = (LogProtoHTTPScraperResponder *)s;
  GString *stats = NULL;

  g_mutex_lock(_mutex());
  iv_validate_now();
  time_t now = iv_now.tv_sec;
  if (self->options->scrape_freq_limit && (now - last_scrape_request_time) < self->options->scrape_freq_limit)
    {
      msg_trace("Too frequent scraper requests, ignoring for now",
                evt_tag_int("last-request", now - last_scrape_request_time),
                evt_tag_int("allowed-freq", self->options->scrape_freq_limit));
      g_mutex_unlock(_mutex());
      /* NOTE: Using an empty response body with the close_after_send=TRUEoption
       *       prevents the scraper waiting for the response till its own timeout */
      return g_string_new("");
    }
  last_scrape_request_time = now;
  g_mutex_unlock(_mutex());

  // Once we have more scraspers to handle these should go into a separate class instead
  switch (self->options->scraper_type)
    {
    case SCT_PROMETHEUS:
      stats = _compose_prometheus_response_body(s);
      break;
    default:
      g_assert(FALSE && "Unknown scraper type");
      break;
    }

  return stats;
}

static gboolean
_check_prometheus_request_headers(LogProtoHTTPServer *s, gchar *buffer_start, gsize buffer_bytes)
{
  // TODO: add a generic header pareser to LogProtoHTTPServer and use it here
  gchar **lines = g_strsplit(buffer_start, "\r\n", 2);

  // First line must be like 'GET /metrics HTTP/1.1\x0d\x0a'
  gchar *line = lines && lines[0] ? lines[0] : buffer_start;
  gchar **tokens = g_strsplit(line, " ", 3);

  gboolean broken = (tokens == NULL || tokens[0] == NULL || strcmp(tokens[0], "GET")
                     || tokens[1] == NULL || strcmp(tokens[1], "/metrics"));

  // TODO: Check further headers as well to support options like compression, etc.
  if (broken)
    msg_error("Unknown request", evt_tag_str("http-scraper-responder", buffer_start));

  if (tokens)
    g_strfreev(tokens);
  if (lines)
    g_strfreev(lines);
  return FALSE == broken;
}

static gboolean
_check_request_headers(LogProtoHTTPServer *s, gchar *buffer_start, gsize buffer_bytes)
{
  LogProtoHTTPScraperResponder *self = (LogProtoHTTPScraperResponder *)s;
  gboolean result = FALSE;

  // Once we have more scraspers to handle these should go into a separate class instead
  switch (self->options->scraper_type)
    {
    case SCT_PROMETHEUS:
      result = _check_prometheus_request_headers(s, buffer_start, buffer_bytes);
      break;
    default:
      g_assert(FALSE && "Unknown scraper type");
      break;
    }
  return result;
}

static void
_log_proto_http_scraper_responder_server_free(LogProtoServer *s)
{
  g_mutex_lock(_mutex());
  LogProtoHTTPScraperResponder *self = (LogProtoHTTPScraperResponder *)s;
  if (self->options->single_instance)
    single_instance = NULL;
  g_mutex_unlock(_mutex());
}

void
log_proto_http_scraper_responder_server_init(LogProtoHTTPScraperResponder *self, LogTransport *transport,
                                             const LogProtoHTTPScraperResponderOptionsStorage *options)
{
  log_proto_http_server_init((LogProtoHTTPServer *)self, transport,
                             (LogProtoHTTPServerOptionsStorage *)options);
  self->super.super.super.super.free_fn = _log_proto_http_scraper_responder_server_free;
  self->super.super.super.no_multi_read = TRUE;
  self->options = (const LogProtoHTTPScraperResponderOptions *)options;

  self->super.request_header_checker = _check_request_headers;
  self->super.response_body_composer = _compose_response_body;
}

LogProtoServer *
log_proto_http_scraper_responder_server_new(LogTransport *transport,
                                            const LogProtoServerOptionsStorage *options)
{
  g_mutex_lock(_mutex());

  LogProtoHTTPScraperResponderOptionsStorage *server_options = (LogProtoHTTPScraperResponderOptionsStorage *)options;
  if (server_options->super.single_instance && single_instance)
    {
      msg_trace("Only one Prometheus scraper responder instance is allowed");
      g_mutex_unlock(_mutex());
      return NULL;
    }

  LogProtoHTTPScraperResponder *self = g_new0(LogProtoHTTPScraperResponder, 1);
  if (server_options->super.single_instance)
    single_instance = self;
  log_proto_http_scraper_responder_server_init(self, transport, server_options);

  g_mutex_unlock(_mutex());
  return &self->super.super.super.super;
}

/*-----------------  Options  -----------------*/

void
log_proto_http_scraper_responder_options_defaults(LogProtoHTTPScraperResponderOptionsStorage *options)
{
  log_proto_http_server_options_defaults((LogProtoHTTPServerOptionsStorage *)&options->storage);
  log_proto_http_server_options_set_close_after_send((LogProtoHTTPServerOptionsStorage *)&options->storage, TRUE);

  options->super.stat_type = 0;
  options->super.scraper_type = 0;
  options->super.scrape_freq_limit = -1;
}

void
log_proto_http_scraper_responder_options_init(LogProtoHTTPScraperResponderOptionsStorage *options,
                                              GlobalConfig *cfg)
{
  if (options->super.initialized)
    return;

  log_proto_http_server_options_init((LogProtoHTTPServerOptionsStorage *)&options->storage, cfg);
  log_proto_http_server_options_set_close_after_send((LogProtoHTTPServerOptionsStorage *)&options->storage, TRUE);

  if (options->super.stat_type == 0)
    options->super.stat_type = STT_STAT;
  if (options->super.scraper_type == 0)
    options->super.scraper_type = SCT_PROMETHEUS;
  if (options->super.scrape_freq_limit == -1)
    options->super.scrape_freq_limit = 0;

  options->super.initialized = TRUE;
}

void
log_proto_http_scraper_responder_options_destroy(LogProtoHTTPScraperResponderOptionsStorage *options)
{
  log_proto_http_server_options_destroy((LogProtoHTTPServerOptionsStorage *)&options->storage);
  g_free(options->super.stat_query);
  options->super.initialized = FALSE;
}

gboolean
log_proto_http_scraper_responder_options_validate(LogProtoHTTPScraperResponderOptionsStorage *options)
{
  if (options->super.stat_type == STT_STAT || options->super.stat_type == STT_QUERY)
    {
      msg_error("prometheus-scraper-response(), type must be 'stat' or 'query'");
      return FALSE;
    }

  return TRUE;
}

gboolean
log_proto_http_scraper_responder_options_set_scraper_type(LogProtoHTTPScraperResponderOptionsStorage *options,
                                                          const gchar *value)
{
  if (strcmp(value, "prometheus") == 0)
    {
      options->super.scraper_type = SCT_PROMETHEUS;
      return TRUE;
    }
  return FALSE;
}

void
log_proto_http_scraper_responder_options_set_scrape_freq_limit(
  LogProtoHTTPScraperResponderOptionsStorage *options,
  gint value)
{
  options->super.scrape_freq_limit = value;
}

void
log_proto_http_scraper_responder_options_set_single_instance(
  LogProtoHTTPScraperResponderOptionsStorage *options,
  gboolean value)
{
  options->super.single_instance = value;
}

gboolean
log_proto_http_scraper_responder_options_set_stat_type(LogProtoHTTPScraperResponderOptionsStorage *options,
                                                       const gchar *value)
{
  if (strcmp(value, "stat") == 0)
    {
      options->super.stat_type = STT_STAT;
      return TRUE;
    }
  else if (strcmp(value, "query") == 0)
    {
      options->super.stat_type = STT_QUERY;
      return TRUE;
    }
  return FALSE;
}

void
log_proto_http_scraper_responder_options_set_stat_query(LogProtoHTTPScraperResponderOptionsStorage *options,
                                                        const gchar *value)
{
  g_free(options->super.stat_query);
  options->super.stat_query = g_strdup(value);
}
