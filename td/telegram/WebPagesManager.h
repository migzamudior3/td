//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2023
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/DialogId.h"
#include "td/telegram/files/FileId.h"
#include "td/telegram/files/FileSourceId.h"
#include "td/telegram/FullMessageId.h"
#include "td/telegram/SecretInputMedia.h"
#include "td/telegram/StoryFullId.h"
#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"
#include "td/telegram/UserId.h"
#include "td/telegram/WebPageId.h"

#include "td/actor/actor.h"
#include "td/actor/MultiTimeout.h"

#include "td/utils/common.h"
#include "td/utils/FlatHashMap.h"
#include "td/utils/FlatHashSet.h"
#include "td/utils/Promise.h"
#include "td/utils/Slice.h"
#include "td/utils/Status.h"
#include "td/utils/WaitFreeHashMap.h"

#include <utility>

namespace td {

struct BinlogEvent;

class Td;

class WebPagesManager final : public Actor {
 public:
  WebPagesManager(Td *td, ActorShared<> parent);

  WebPagesManager(const WebPagesManager &) = delete;
  WebPagesManager &operator=(const WebPagesManager &) = delete;
  WebPagesManager(WebPagesManager &&) = delete;
  WebPagesManager &operator=(WebPagesManager &&) = delete;
  ~WebPagesManager() final;

  WebPageId on_get_web_page(tl_object_ptr<telegram_api::WebPage> &&web_page_ptr, DialogId owner_dialog_id);

  void on_get_web_page_by_url(const string &url, WebPageId web_page_id, bool from_database);

  void on_get_web_page_instant_view_view_count(WebPageId web_page_id, int32 view_count);

  void register_web_page(WebPageId web_page_id, FullMessageId full_message_id, const char *source);

  void unregister_web_page(WebPageId web_page_id, FullMessageId full_message_id, const char *source);

  bool have_web_page(WebPageId web_page_id) const;

  bool have_web_page_force(WebPageId web_page_id);

  tl_object_ptr<td_api::webPage> get_web_page_object(WebPageId web_page_id) const;

  tl_object_ptr<td_api::webPageInstantView> get_web_page_instant_view_object(WebPageId web_page_id) const;

  void get_web_page_preview(td_api::object_ptr<td_api::formattedText> &&text,
                            Promise<td_api::object_ptr<td_api::webPage>> &&promise);

  void get_web_page_instant_view(const string &url, bool force_full, Promise<WebPageId> &&promise);

  WebPageId get_web_page_by_url(const string &url) const;

  void get_web_page_by_url(const string &url, Promise<WebPageId> &&promise);

  void reload_web_page_by_url(const string &url, Promise<WebPageId> &&promise);

  void on_get_web_page_preview(const string &url, tl_object_ptr<telegram_api::MessageMedia> &&message_media_ptr,
                               Promise<td_api::object_ptr<td_api::webPage>> &&promise);

  SecretInputMedia get_secret_input_media(WebPageId web_page_id) const;

  void on_binlog_web_page_event(BinlogEvent &&event);

  FileSourceId get_url_file_source_id(const string &url);

  string get_web_page_search_text(WebPageId web_page_id) const;

  int32 get_web_page_media_duration(WebPageId web_page_id) const;

  StoryFullId get_web_page_story_full_id(WebPageId web_page_id) const;

  vector<UserId> get_web_page_user_ids(WebPageId web_page_id) const;

  void on_story_changed(StoryFullId story_full_id);

 private:
  class WebPage;

  class WebPageInstantView;

  class WebPageLogEvent;

  void update_web_page(unique_ptr<WebPage> web_page, WebPageId web_page_id, bool from_binlog, bool from_database);

  void update_web_page_instant_view(WebPageId web_page_id, WebPageInstantView &new_instant_view,
                                    WebPageInstantView &&old_instant_view);

  static bool need_use_old_instant_view(const WebPageInstantView &new_instant_view,
                                        const WebPageInstantView &old_instant_view);

  void on_web_page_changed(WebPageId web_page_id, bool have_web_page);

  const WebPage *get_web_page(WebPageId web_page_id) const;

  const WebPageInstantView *get_web_page_instant_view(WebPageId web_page_id) const;

  void get_web_page_instant_view_impl(WebPageId web_page_id, bool force_full, Promise<WebPageId> &&promise);

  tl_object_ptr<td_api::webPageInstantView> get_web_page_instant_view_object(
      WebPageId web_page_id, const WebPageInstantView *web_page_instant_view, Slice web_page_url) const;

  static void on_pending_web_page_timeout_callback(void *web_pages_manager_ptr, int64 web_page_id_int);

  void on_pending_web_page_timeout(WebPageId web_page_id);

  void on_get_web_page_preview_success(const string &url, WebPageId web_page_id,
                                       Promise<td_api::object_ptr<td_api::webPage>> &&promise);

  void on_get_web_page_instant_view(WebPage *web_page, tl_object_ptr<telegram_api::page> &&page, int32 hash,
                                    DialogId owner_dialog_id);

  void save_web_page(const WebPage *web_page, WebPageId web_page_id, bool from_binlog);

  static string get_web_page_database_key(WebPageId web_page_id);

  void on_save_web_page_to_database(WebPageId web_page_id, bool success);

  void load_web_page_from_database(WebPageId web_page_id, Promise<Unit> promise);

  void on_load_web_page_from_database(WebPageId web_page_id, string value);

  const WebPage *get_web_page_force(WebPageId web_page_id);

  static string get_web_page_instant_view_database_key(WebPageId web_page_id);

  void load_web_page_instant_view(WebPageId web_page_id, bool force_full, Promise<WebPageId> &&promise);

  void on_load_web_page_instant_view_from_database(WebPageId web_page_id, string value);

  void reload_web_page_instant_view(WebPageId web_page_id);

  void update_web_page_instant_view_load_requests(WebPageId web_page_id, bool force_update,
                                                  Result<WebPageId> r_web_page_id);

  static string get_web_page_url_database_key(const string &url);

  void load_web_page_by_url(string url, Promise<WebPageId> &&promise);

  void on_load_web_page_id_by_url_from_database(string url, string value, Promise<WebPageId> &&promise);

  void on_load_web_page_by_url_from_database(WebPageId web_page_id, string url, Promise<WebPageId> &&promise,
                                             Result<Unit> &&result);

  void tear_down() final;

  int32 get_web_page_media_duration(const WebPage *web_page) const;

  FileSourceId get_web_page_file_source_id(WebPage *web_page);

  vector<FileId> get_web_page_file_ids(const WebPage *web_page) const;

  Td *td_;
  ActorShared<> parent_;
  WaitFreeHashMap<WebPageId, unique_ptr<WebPage>, WebPageIdHash> web_pages_;

  FlatHashMap<WebPageId, vector<Promise<Unit>>, WebPageIdHash> load_web_page_from_database_queries_;
  FlatHashSet<WebPageId, WebPageIdHash> loaded_from_database_web_pages_;

  struct PendingWebPageInstantViewQueries {
    vector<Promise<WebPageId>> partial;
    vector<Promise<WebPageId>> full;
  };
  FlatHashMap<WebPageId, PendingWebPageInstantViewQueries, WebPageIdHash> load_web_page_instant_view_queries_;

  FlatHashMap<WebPageId, FlatHashSet<FullMessageId, FullMessageIdHash>, WebPageIdHash> web_page_messages_;

  FlatHashMap<WebPageId, vector<std::pair<string, Promise<td_api::object_ptr<td_api::webPage>>>>, WebPageIdHash>
      pending_get_web_pages_;

  FlatHashMap<StoryFullId, FlatHashSet<WebPageId, WebPageIdHash>, StoryFullIdHash> story_web_pages_;

  FlatHashMap<string, WebPageId> url_to_web_page_id_;

  FlatHashMap<string, FileSourceId> url_to_file_source_id_;

  MultiTimeout pending_web_pages_timeout_{"PendingWebPagesTimeout"};
};

}  // namespace td
