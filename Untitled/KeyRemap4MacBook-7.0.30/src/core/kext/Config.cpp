#include "Config.hpp"
#include "version.hpp"
#include "Client.hpp"
#include "FlagStatus.hpp"
#include "RemapClass.hpp"
#include "RemapFunc/PointingRelativeToScroll.hpp"
#include "util/CommonData.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  int Config::debug = 0;
  int Config::debug_pointing = 0;
  int Config::debug_devel = 0;
  int Config::initialized = 0;
  int Config::do_reset = 0;
  int Config::do_reload_xml = 0;
  int Config::do_reload_only_config = 0;
  char Config::socket_path[SOCKET_PATH_MAX];

  int Config::essential_config_[BRIDGE_ESSENTIAL_CONFIG_INDEX__END__] = {
#include "../bridge/config/output/include.bridge_essential_config_index.cpp"
  };

  const int Config::essential_config_default_[BRIDGE_ESSENTIAL_CONFIG_INDEX__END__] = {
#include "../bridge/config/output/include.bridge_essential_config_index.cpp"
  };

  int
  Config::do_reset_handler SYSCTL_HANDLER_ARGS
  {
    IOLockWrapper::ScopedLock lk_eventlock(CommonData::getEventLock());
    if (! lk_eventlock) return EAGAIN;

    int oldvalue = Config::do_reset;

    int error = sysctl_handle_int(oidp, oidp->oid_arg1, oidp->oid_arg2, req);
    if (! error && req->newptr) {
      if (Config::do_reset == 1 && oldvalue != 1) {
        IOLOG_INFO("Config::do_reset\n");

        Config::load_essential_config_default();
        RemapClassManager::clear_xml();

        Config::debug = 0;
        Config::debug_pointing = 0;
        Config::debug_devel = 0;

        Config::socket_path[0] = '\0';
        KeyRemap4MacBook_client::refreshSockAddr();

        Config::do_reset = 0;
        Config::initialized = 0;
      }
    }
    return error;
  }

  int
  Config::do_reload_xml_handler SYSCTL_HANDLER_ARGS
  {
    IOLockWrapper::ScopedLock lk_eventlock(CommonData::getEventLock());
    if (! lk_eventlock) return EAGAIN;

    int oldvalue = Config::do_reload_xml;

    int error = sysctl_handle_int(oidp, oidp->oid_arg1, oidp->oid_arg2, req);
    if (! error && req->newptr) {
      if (Config::do_reload_xml == 1 && oldvalue != 1) {
        IOLOG_INFO("Config::do_reload_xml\n");

        Config::load_essential_config();
        if (RemapClassManager::reload_xml()) {
          Config::do_reload_xml = 0;
          Config::initialized = 1;
        } else {
          IOLOG_ERROR("RemapClassManager::reload_xml is failed.\n");
          Config::do_reload_xml = -1;
          Config::initialized = 0;
        }
      }
    }
    return error;
  }

  int
  Config::do_reload_only_config_handler SYSCTL_HANDLER_ARGS
  {
    IOLockWrapper::ScopedLock lk_eventlock(CommonData::getEventLock());
    if (! lk_eventlock) return EAGAIN;

    int oldvalue = Config::do_reload_only_config;

    int error = sysctl_handle_int(oidp, oidp->oid_arg1, oidp->oid_arg2, req);
    if (! error && req->newptr) {
      if (Config::do_reload_only_config == 1 && oldvalue != 1) {
        // When using ThreeFingerRelativeToScroll,
        // Config::do_reload_only_config called by each Scroll Switching.
        // So, suppress IOLOG_INFO in here.

        // IOLOG_INFO("Config::do_reload_only_config\n");

        Config::load_essential_config();
        if (RemapClassManager::reload_xml()) {
          Config::do_reload_only_config = 0;
        } else {
          IOLOG_ERROR("RemapClassManager::reload_xml is failed.\n");
          Config::do_reload_only_config = -1;
        }
      }
    }
    return error;
  }

  int
  Config::socket_path_handler SYSCTL_HANDLER_ARGS
  {
    int error = sysctl_handle_string(oidp, oidp->oid_arg1, oidp->oid_arg2, req);
    if (! error && req->newptr) {
      IOLOG_INFO("Config::socket_path_handler\n");

      KeyRemap4MacBook_client::refreshSockAddr();
    }
    return error;
  }

  // ----------------------------------------------------------------------
  // SYSCTL staff
  // http://developer.apple.com/documentation/Darwin/Conceptual/KernelProgramming/boundaries/chapter_14_section_7.html#//apple_ref/doc/uid/TP30000905-CH217-TPXREF116
  SYSCTL_DECL(_keyremap4macbook);
  SYSCTL_NODE(, OID_AUTO, keyremap4macbook, CTLFLAG_RW, 0, "");

  // ----------------------------------------
  SYSCTL_INT(_keyremap4macbook,    OID_AUTO, initialized,           CTLTYPE_INT,                 &(Config::initialized),           0,                                        "");
  SYSCTL_INT(_keyremap4macbook,    OID_AUTO, debug,                 CTLTYPE_INT | CTLFLAG_RW,    &(Config::debug),                 0,                                        "");
  SYSCTL_INT(_keyremap4macbook,    OID_AUTO, debug_pointing,        CTLTYPE_INT | CTLFLAG_RW,    &(Config::debug_pointing),        0,                                        "");
  SYSCTL_INT(_keyremap4macbook,    OID_AUTO, debug_devel,           CTLTYPE_INT | CTLFLAG_RW,    &(Config::debug_devel),           0,                                        "");
  SYSCTL_STRING(_keyremap4macbook, OID_AUTO, version,               CTLFLAG_RD,                  config_version,                   0,                                        "");
  SYSCTL_PROC(_keyremap4macbook,   OID_AUTO, do_reset,              CTLTYPE_INT | CTLFLAG_RW,    &(Config::do_reset),              0, Config::do_reset_handler,                 "I", "");
  SYSCTL_PROC(_keyremap4macbook,   OID_AUTO, do_reload_xml,         CTLTYPE_INT | CTLFLAG_RW,    &(Config::do_reload_xml),         0, Config::do_reload_xml_handler,            "I", "");
  SYSCTL_PROC(_keyremap4macbook,   OID_AUTO, do_reload_only_config, CTLTYPE_INT | CTLFLAG_RW,    &(Config::do_reload_only_config), 0, Config::do_reload_only_config_handler,    "I", "");
  SYSCTL_PROC(_keyremap4macbook,   OID_AUTO, socket_path,           CTLTYPE_STRING | CTLFLAG_RW, Config::socket_path, sizeof(Config::socket_path), Config::socket_path_handler, "A", "");

  // ----------------------------------------------------------------------
  void
  Config::initialize(void)
  {
    socket_path[0] = '\0';
  }

  void
  Config::terminate(void)
  {}

  void
  Config::sysctl_register(void)
  {
    sysctl_register_oid(&sysctl__keyremap4macbook);

    sysctl_register_oid(&sysctl__keyremap4macbook_socket_path);
    sysctl_register_oid(&sysctl__keyremap4macbook_debug);
    sysctl_register_oid(&sysctl__keyremap4macbook_debug_pointing);
    sysctl_register_oid(&sysctl__keyremap4macbook_debug_devel);
    sysctl_register_oid(&sysctl__keyremap4macbook_version);
    sysctl_register_oid(&sysctl__keyremap4macbook_initialized);
    sysctl_register_oid(&sysctl__keyremap4macbook_do_reset);
    sysctl_register_oid(&sysctl__keyremap4macbook_do_reload_xml);
    sysctl_register_oid(&sysctl__keyremap4macbook_do_reload_only_config);
  }

  void
  Config::sysctl_unregister(void)
  {
    sysctl_unregister_oid(&sysctl__keyremap4macbook);

    sysctl_unregister_oid(&sysctl__keyremap4macbook_socket_path);
    sysctl_unregister_oid(&sysctl__keyremap4macbook_debug);
    sysctl_unregister_oid(&sysctl__keyremap4macbook_debug_pointing);
    sysctl_unregister_oid(&sysctl__keyremap4macbook_debug_devel);
    sysctl_unregister_oid(&sysctl__keyremap4macbook_version);
    sysctl_unregister_oid(&sysctl__keyremap4macbook_initialized);
    sysctl_unregister_oid(&sysctl__keyremap4macbook_do_reset);
    sysctl_unregister_oid(&sysctl__keyremap4macbook_do_reload_xml);
    sysctl_unregister_oid(&sysctl__keyremap4macbook_do_reload_only_config);
  }

  void
  Config::load_essential_config_default(void)
  {
    for (int i = 0; i < BRIDGE_ESSENTIAL_CONFIG_INDEX__END__; ++i) {
      essential_config_[i] = essential_config_default_[i];
    }
  }

  void
  Config::load_essential_config(void)
  {
    // load_essential_config is called from one thread.
    // So, we can use static local variable to reduce stack usage.
    static KeyRemap4MacBook_bridge::GetEssentialConfig::Reply reply;
    time_t timeout_second = 3;
    int error = KeyRemap4MacBook_client::sendmsg(KeyRemap4MacBook_bridge::REQUEST_GET_ESSENTIAL_CONFIG, NULL, 0, &reply, sizeof(reply), timeout_second, 0);
    if (error) {
      IOLOG_ERROR("do_reload_xml GetEssentialConfig sendmsg failed. (%d)\n", error);
      return;
    }
    for (size_t i = 0; i < sizeof(reply.value) / sizeof(reply.value[0]); ++i) {
      essential_config_[i] = reply.value[i];
    }

    // ------------------------------------------------------------
    // reset values
    FlagStatus::lock_clear();
    FlagStatus::sticky_clear();
    RemapClassManager::refresh();
    RemapFunc::PointingRelativeToScroll::cancelScroll();
  }
}
