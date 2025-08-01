/*
 * Copyright (c) 2025 One Identity
 * Copyright (c) 2025 Hofi <hofione@gmail.com>
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
#ifndef LOGPROTO_HTTP_SCRAPER_RESPONDER_INCLUDED
#define LOGPROTO_HTTP_SCRAPER_RESPONDER_INCLUDED

#include "logproto/logproto-http-server.h"

/* Stat type flags */
#define STT_STATS  0x01
#define STT_QUERY 0x02

/* options */
typedef struct _LogProtoHTTPScraperResponderOptions
{
  LogProtoHTTPServerOptions super;
  gchar *scraper_request_hdr_pattern;
  gint scrape_freq_limit;
  guint8 stat_type;
  gchar *stat_query;
  gchar *stat_format;
  gboolean single_instance;
  gboolean stats_without_orphaned;
  gboolean stats_with_legacy;

} LogProtoHTTPScraperResponderOptions;

typedef union _LogProtoHTTPScraperResponderOptionsStoreage
{
  LogProtoServerOptionsStorage storage;
  LogProtoHTTPScraperResponderOptions super;

} LogProtoHTTPScraperResponderOptionsStorage;
// _Static_assert() is a C11 feature, so we use a typedef trick to perform the static assertion
typedef char static_assert_size_check_LogProtoHTTPScraperResponderOptions[
   sizeof(LogProtoServerOptionsStorage) >= sizeof(LogProtoHTTPScraperResponderOptions) ? 1 : -1];

/* LogProtoHTTPScraperResponder */
typedef struct _LogProtoHTTPScraperResponder LogProtoHTTPScraperResponder;
struct _LogProtoHTTPScraperResponder
{
  LogProtoHTTPServer super;
  const LogProtoHTTPScraperResponderOptions *options;
};

void log_proto_http_scraper_responder_options_defaults(LogProtoServerOptionsStorage *options);
void log_proto_http_scraper_responder_options_init(LogProtoServerOptionsStorage *options,
                                                   GlobalConfig *cfg);
void log_proto_http_scraper_responder_options_destroy(LogProtoServerOptionsStorage *options);
gboolean log_proto_http_scraper_responder_options_validate(LogProtoServerOptionsStorage *options);

void log_proto_http_scraper_responder_options_set_scrape_pattern(
  LogProtoServerOptionsStorage *options_storage,
  const gchar *value);
void log_proto_http_scraper_responder_options_set_scrape_freq_limit(
  LogProtoServerOptionsStorage *options,
  gint value);
void log_proto_http_scraper_responder_options_set_single_instance(
  LogProtoServerOptionsStorage *options,
  gboolean value);
gboolean log_proto_http_scraper_responder_options_set_stat_type(
  LogProtoServerOptionsStorage *options,
  const gchar *value);
void log_proto_http_scraper_responder_options_set_stat_query(
  LogProtoServerOptionsStorage *options,
  const gchar *value);
gboolean log_proto_http_scraper_responder_options_set_stat_format(
  LogProtoServerOptionsStorage *options,
  const gchar *value);
void log_proto_http_scraper_responder_options_set_stats_without_orpaned(
  LogProtoServerOptionsStorage *options_storage,
  gboolean value);
void log_proto_http_scraper_responder_options_set_stats_with_legacy(
  LogProtoServerOptionsStorage *options_storage,
  gboolean value);

LogProtoServer *log_proto_http_scraper_responder_server_new(
  LogTransport *transport,
  const LogProtoServerOptionsStorage *options);
void log_proto_http_scraper_responder_server_init(LogProtoHTTPScraperResponder *self,
                                                  LogTransport *transport,
                                                  const LogProtoServerOptionsStorage *options);

#endif
