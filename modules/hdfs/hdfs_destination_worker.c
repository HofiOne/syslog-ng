/*
 * Copyright (c) 2023 One Identity LLC.
 * Copyright (c) 2020 Balabit
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */
#include "hdfs_destination_worker.h"
#include "hdfs_destination.h"
#include "thread-utils.h"

#include <stdio.h>
#include "hdfs/hdfs.h"

#define LOG_PREFIX "HDFS - "

typedef struct _HDFSDestinationWorker
{
  LogThreadedDestWorker super;
  hdfsFS hdfs;
  hdfsFile file;
  ThreadId thread_id;
} HDFSDestinationWorker;

static LogThreadedResult
_dw_insert(LogThreadedDestWorker *s, LogMessage *msg)
{
  HDFSDestinationWorker *self = (HDFSDestinationWorker *)s;

  GString *string_to_write = g_string_new("");
  g_string_printf(string_to_write, "thread_id=%lu message=%s\n",
                  self->thread_id, log_msg_get_value(msg, LM_V_MESSAGE, NULL));

  gsize string_to_write_len = string_to_write->len;
  size_t retval = hdfsWrite(self->hdfs, self->file, string_to_write->str, string_to_write_len);
  g_string_free(string_to_write, TRUE);

  if (retval != string_to_write_len)
    {
      msg_error(LOG_PREFIX "Error while writing file",
                evt_tag_str("lasterror", (const gchar *)hdfsGetLastError()));

      return LTR_NOT_CONNECTED;
    }

  msg_debug(LOG_PREFIX "Log inserted");

  return LTR_SUCCESS;
  /*
   * LTR_DROP,
   * LTR_ERROR,
   * LTR_SUCCESS,
   * LTR_QUEUED,
   * LTR_NOT_CONNECTED,
   * LTR_RETRY,
  */
}

static LogThreadedResult
_dw_flush(LogThreadedDestWorker *s, LogThreadedFlushMode mode)
{
  return LTR_SUCCESS;

  HDFSDestinationWorker *self = (HDFSDestinationWorker *)s;

  if (hdfsFileIsOpenForWrite(self->file))
    {
      if (hdfsFlush(self->hdfs, self->file) != 0)
        {
          msg_error(LOG_PREFIX "Error while flushing file",
                    evt_tag_str("lasterror", (const gchar *)hdfsGetLastError()));
          return LTR_NOT_CONNECTED;
        }

      if (hdfsSync(self->hdfs, self->file) != 0)
        {
          msg_error(LOG_PREFIX "Error while syncing file",
                    evt_tag_str("lasterror", (const gchar *)hdfsGetLastError()));
          return LTR_NOT_CONNECTED;
        }
    }
  return LTR_SUCCESS;
  /*
   * LTR_DROP,
   * LTR_ERROR,
   * LTR_SUCCESS,
   * LTR_QUEUED,
   * LTR_NOT_CONNECTED,
   * LTR_RETRY,
  */
}


static gboolean
_connect(LogThreadedDestWorker *s)
{
  HDFSDestinationWorker *self = (HDFSDestinationWorker *)s;
  HDFSDestinationDriver *owner = (HDFSDestinationDriver *)s->owner;

  msg_debug(LOG_PREFIX "Connecing to HDFS...");

  //self->hdfs = hdfsConnect("192.168.1.111", 9000);
  //self->hdfs = hdfsConnect("hdfs://10.12.129.40", 8020);
  self->hdfs = hdfsConnect("hdfs://hdp2.syslog-ng.balabit", 8020);

  //self->hdfs = hdfsConnect("hdfs://hadoop-host-1.ssb.balabit", 8020);
  //self->hdfs = hdfsConnectAsUser("hdfs://hadoop-host-1.ssb.balabit", 8020, "admin");
  //
  // struct hdfsBuilder *bld = hdfsNewBuilder();
  // hdfsBuilderSetNameNode(bld, "hdfs://hadoop-host-2.ssb.balabit");
  // hdfsBuilderSetNameNodePort(bld, 8020);
  // //hdfsBuilderSetUserName(bld, "admin");
  // self->hdfs = hdfsBuilderConnect(bld);
  // hdfsFreeBuilder(bld);


  if (self->hdfs == NULL)
    {
      msg_error(LOG_PREFIX "Could not connect to HDFS",
                evt_tag_str("lasterror", (const gchar *)hdfsGetLastError()));
      return FALSE;
    }

  gchar *path = owner->filename->str;
  gchar *dirname = g_path_get_dirname(path);
  hdfsCreateDirectory(self->hdfs, dirname);
  g_free(dirname);

  gboolean exists = hdfsExists(self->hdfs, path);
  self->file = hdfsOpenFile(self->hdfs, path, O_WRONLY|(exists ? O_APPEND : 0), 0, 1, 0);
  if (self->file == NULL)
    {
      msg_error(LOG_PREFIX "Could not open HDFS file",
                evt_tag_str("lasterror", (const gchar *)hdfsGetLastError()));
      return FALSE;
    }

  msg_debug(LOG_PREFIX "Connected to HDFS");

  return TRUE;
}

static void
_disconnect(LogThreadedDestWorker *s)
{
  HDFSDestinationWorker *self = (HDFSDestinationWorker *)s;

  hdfsCloseFile(self->hdfs, self->file);
  hdfsDisconnect(self->hdfs);
  self->hdfs = NULL;
}

static gboolean
_dw_init(LogThreadedDestWorker *s)
{
  HDFSDestinationWorker *self = (HDFSDestinationWorker *)s;

  /*
    You can create thread specific resources here. In this example, we
    store the thread id.
  */
  self->thread_id = get_thread_id();

  return log_threaded_dest_worker_init_method(s);
}

static void
_dw_deinit(LogThreadedDestWorker *s)
{
  /*
    If you created resources during _thread_init,
    you need to free them here
  */

  log_threaded_dest_worker_deinit_method(s);
}

static void
_dw_free(LogThreadedDestWorker *s)
{
  /*
    If you created resources during new,
    you need to free them here.
  */

  log_threaded_dest_worker_free_method(s);
}

LogThreadedDestWorker *
hdfs_destination_dw_new(LogThreadedDestDriver *o, gint worker_index)
{
  HDFSDestinationWorker *self = g_new0(HDFSDestinationWorker, 1);

  log_threaded_dest_worker_init_instance(&self->super, o, worker_index);
  self->super.batch_size = 1;

  self->super.init = _dw_init;
  self->super.deinit = _dw_deinit;
  self->super.insert = _dw_insert;
  self->super.flush = _dw_flush;
  self->super.free_fn = _dw_free;
  self->super.connect = _connect;
  self->super.disconnect = _disconnect;

  return &self->super;
}
