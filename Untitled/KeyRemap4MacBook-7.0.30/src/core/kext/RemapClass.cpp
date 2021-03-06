#include "bridge.h"
#include "RemapClass.hpp"
#include "Client.hpp"
#include "KeyboardRepeat.hpp"
#include "VirtualKey.hpp"
#include "util/CommonData.hpp"
#include "util/EventInputQueue.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  RemapClass::Item::Item(const unsigned int* vec, size_t length)
  {
    type_ = BRIDGE_REMAPTYPE_NONE;
    filters_ = NULL;

    // ------------------------------------------------------------
    // check parameters.
    //
    if (! vec || length <= 0) {
      IOLOG_ERROR("RemapClass::Item::Item invalid parameter.\n");
      return;
    }

    // ------------------------------------------------------------
    // initialize values.
    //
#define INITIALIZE_UNION_VALUE(POINTER, CLASS) {                   \
    POINTER = new CLASS;                                           \
    if (! POINTER) {                                               \
      IOLOG_ERROR("RemapClass::Item::Item failed to allocate.\n"); \
      return;                                                      \
    } else {                                                       \
      type_ = newtype;                                             \
      for (size_t i = 1;; i += 2) {                                \
        size_t datatype_index = i;                                 \
        size_t value_index    = i + 1;                             \
        if (value_index >= length) break;                          \
        (POINTER)->add(vec[datatype_index], vec[value_index]);     \
      }                                                            \
    }                                                              \
}

    unsigned int newtype = vec[0];

    switch (newtype) {
      // handle BRIDGE_REMAPTYPE_NONE as error. (see default)
      case BRIDGE_REMAPTYPE_KEYTOKEY:                       INITIALIZE_UNION_VALUE(p_.keyToKey,                       RemapFunc::KeyToKey);                       break;
      case BRIDGE_REMAPTYPE_KEYTOCONSUMER:                  INITIALIZE_UNION_VALUE(p_.keyToConsumer,                  RemapFunc::KeyToConsumer);                  break;
      case BRIDGE_REMAPTYPE_KEYTOPOINTINGBUTTON:            INITIALIZE_UNION_VALUE(p_.keyToPointingButton,            RemapFunc::KeyToPointingButton);            break;
      case BRIDGE_REMAPTYPE_CONSUMERTOCONSUMER:             INITIALIZE_UNION_VALUE(p_.consumerToConsumer,             RemapFunc::ConsumerToConsumer);             break;
      case BRIDGE_REMAPTYPE_CONSUMERTOKEY:                  INITIALIZE_UNION_VALUE(p_.consumerToKey,                  RemapFunc::ConsumerToKey);                  break;
      case BRIDGE_REMAPTYPE_DOUBLEPRESSMODIFIER:            INITIALIZE_UNION_VALUE(p_.doublePressModifier,            RemapFunc::DoublePressModifier);            break;
      case BRIDGE_REMAPTYPE_DROPKEYAFTERREMAP:              INITIALIZE_UNION_VALUE(p_.dropKeyAfterRemap,              RemapFunc::DropKeyAfterRemap);              break;
      case BRIDGE_REMAPTYPE_DROPPOINTINGRELATIVECURSORMOVE: INITIALIZE_UNION_VALUE(p_.dropPointingRelativeCursorMove, RemapFunc::DropPointingRelativeCursorMove); break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:                INITIALIZE_UNION_VALUE(p_.holdingKeyToKey,                RemapFunc::HoldingKeyToKey);                break;
      case BRIDGE_REMAPTYPE_IGNOREMULTIPLESAMEKEYPRESS:     INITIALIZE_UNION_VALUE(p_.ignoreMultipleSameKeyPress,     RemapFunc::IgnoreMultipleSameKeyPress);     break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER:            INITIALIZE_UNION_VALUE(p_.keyOverlaidModifier,            RemapFunc::KeyOverlaidModifier);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOKEY:            INITIALIZE_UNION_VALUE(p_.pointingButtonToKey,            RemapFunc::PointingButtonToKey);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOPOINTINGBUTTON: INITIALIZE_UNION_VALUE(p_.pointingButtonToPointingButton, RemapFunc::PointingButtonToPointingButton); break;
      case BRIDGE_REMAPTYPE_POINTINGRELATIVETOSCROLL:       INITIALIZE_UNION_VALUE(p_.pointingRelativeToScroll,       RemapFunc::PointingRelativeToScroll);       break;
      case BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES:         INITIALIZE_UNION_VALUE(p_.simultaneousKeyPresses,         RemapFunc::SimultaneousKeyPresses);         break;
      case BRIDGE_REMAPTYPE_SETKEYBOARDTYPE:                INITIALIZE_UNION_VALUE(p_.setKeyboardType,                RemapFunc::SetKeyboardType);                break;
      default:
        IOLOG_ERROR("RemapClass::Item::Item unknown type_ (%d)\n", type_);
        type_ = BRIDGE_REMAPTYPE_NONE;
        return;
    }

#undef INITIALIZE_UNION_VALUE
  }

  RemapClass::Item::~Item(void)
  {
#define DELETE_UNLESS_NULL(POINTER) { \
    if (POINTER) { delete POINTER; }  \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_NONE:                                                                                  break;
      case BRIDGE_REMAPTYPE_KEYTOKEY:                       DELETE_UNLESS_NULL(p_.keyToKey);                       break;
      case BRIDGE_REMAPTYPE_KEYTOCONSUMER:                  DELETE_UNLESS_NULL(p_.keyToConsumer);                  break;
      case BRIDGE_REMAPTYPE_KEYTOPOINTINGBUTTON:            DELETE_UNLESS_NULL(p_.keyToPointingButton);            break;
      case BRIDGE_REMAPTYPE_CONSUMERTOCONSUMER:             DELETE_UNLESS_NULL(p_.consumerToConsumer);             break;
      case BRIDGE_REMAPTYPE_CONSUMERTOKEY:                  DELETE_UNLESS_NULL(p_.consumerToKey);                  break;
      case BRIDGE_REMAPTYPE_DOUBLEPRESSMODIFIER:            DELETE_UNLESS_NULL(p_.doublePressModifier);            break;
      case BRIDGE_REMAPTYPE_DROPKEYAFTERREMAP:              DELETE_UNLESS_NULL(p_.dropKeyAfterRemap);              break;
      case BRIDGE_REMAPTYPE_DROPPOINTINGRELATIVECURSORMOVE: DELETE_UNLESS_NULL(p_.dropPointingRelativeCursorMove); break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:                DELETE_UNLESS_NULL(p_.holdingKeyToKey);                break;
      case BRIDGE_REMAPTYPE_IGNOREMULTIPLESAMEKEYPRESS:     DELETE_UNLESS_NULL(p_.ignoreMultipleSameKeyPress);     break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER:            DELETE_UNLESS_NULL(p_.keyOverlaidModifier);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOKEY:            DELETE_UNLESS_NULL(p_.pointingButtonToKey);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOPOINTINGBUTTON: DELETE_UNLESS_NULL(p_.pointingButtonToPointingButton); break;
      case BRIDGE_REMAPTYPE_POINTINGRELATIVETOSCROLL:       DELETE_UNLESS_NULL(p_.pointingRelativeToScroll);       break;
      case BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES:         DELETE_UNLESS_NULL(p_.simultaneousKeyPresses);         break;
      case BRIDGE_REMAPTYPE_SETKEYBOARDTYPE:                DELETE_UNLESS_NULL(p_.setKeyboardType);                break;
      default:
        IOLOG_ERROR("RemapClass::Item::terminate unknown type_ (%d)\n", type_);
        break;
    }

#undef DELETE_UNLESS_NULL

    // ------------------------------------------------------------
    if (filters_) {
      for (size_t i = 0; i < filters_->size(); ++i) {
        RemapFilter::FilterUnion* p = (*filters_)[i];
        if (p) {
          delete p;
        }
      }
      delete filters_;
    }
  }

  void
  RemapClass::Item::append_filter(const unsigned int* vec, size_t length)
  {
    // ------------------------------------------------------------
    // check parameters.
    //
    if (! vec || length <= 0) {
      IOLOG_ERROR("RemapClass::Item::append_filter invalid parameter.\n");
      return;
    }

    // ------------------------------------------------------------
    // append to filters_.
    //
    if (! filters_) {
      filters_ = new RemapFilter::Vector_FilterUnionPointer();

      if (! filters_) {
        IOLOG_ERROR("RemapClass::Item::append_filter failed to allocate.\n");
        return;
      }
    }

    RemapFilter::FilterUnion* newp = new RemapFilter::FilterUnion(vec, length);
    if (! newp) {
      IOLOG_ERROR("RemapClass::Item::append_filter failed to allocate.\n");
      return;
    }

    filters_->push_back(newp);
  }

  bool
  RemapClass::Item::remap(RemapParams& remapParams)
  {
    if (isblocked()) return false;

#define CALL_UNION_FUNCTION(POINTER) {                     \
    if (POINTER) { return (POINTER)->remap(remapParams); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_KEYTOKEY:                   CALL_UNION_FUNCTION(p_.keyToKey);                   break;
      case BRIDGE_REMAPTYPE_KEYTOCONSUMER:              CALL_UNION_FUNCTION(p_.keyToConsumer);              break;
      case BRIDGE_REMAPTYPE_KEYTOPOINTINGBUTTON:        CALL_UNION_FUNCTION(p_.keyToPointingButton);        break;
      case BRIDGE_REMAPTYPE_DOUBLEPRESSMODIFIER:        CALL_UNION_FUNCTION(p_.doublePressModifier);        break;
      case BRIDGE_REMAPTYPE_HOLDINGKEYTOKEY:            CALL_UNION_FUNCTION(p_.holdingKeyToKey);            break;
      case BRIDGE_REMAPTYPE_IGNOREMULTIPLESAMEKEYPRESS: CALL_UNION_FUNCTION(p_.ignoreMultipleSameKeyPress); break;
      case BRIDGE_REMAPTYPE_KEYOVERLAIDMODIFIER:        CALL_UNION_FUNCTION(p_.keyOverlaidModifier);        break;
      case BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES:     CALL_UNION_FUNCTION(p_.simultaneousKeyPresses);     break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION

    return false;
  }

  bool
  RemapClass::Item::remap(RemapConsumerParams& remapParams)
  {
    if (isblocked()) return false;

#define CALL_UNION_FUNCTION(POINTER) {                     \
    if (POINTER) { return (POINTER)->remap(remapParams); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_CONSUMERTOCONSUMER: CALL_UNION_FUNCTION(p_.consumerToConsumer); break;
      case BRIDGE_REMAPTYPE_CONSUMERTOKEY:      CALL_UNION_FUNCTION(p_.consumerToKey);      break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION

    return false;
  }

  bool
  RemapClass::Item::remap(RemapPointingParams_relative& remapParams)
  {
    if (isblocked()) return false;

#define CALL_UNION_FUNCTION(POINTER) {                     \
    if (POINTER) { return (POINTER)->remap(remapParams); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_DROPPOINTINGRELATIVECURSORMOVE: CALL_UNION_FUNCTION(p_.dropPointingRelativeCursorMove); break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOKEY:            CALL_UNION_FUNCTION(p_.pointingButtonToKey);            break;
      case BRIDGE_REMAPTYPE_POINTINGBUTTONTOPOINTINGBUTTON: CALL_UNION_FUNCTION(p_.pointingButtonToPointingButton); break;
      case BRIDGE_REMAPTYPE_POINTINGRELATIVETOSCROLL:       CALL_UNION_FUNCTION(p_.pointingRelativeToScroll);       break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION

    return false;
  }

  bool
  RemapClass::Item::drop(const Params_KeyboardEventCallBack& params)
  {
    if (isblocked()) return false;

#define CALL_UNION_FUNCTION(POINTER) {               \
    if (POINTER) { return (POINTER)->drop(params); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_DROPKEYAFTERREMAP: CALL_UNION_FUNCTION(p_.dropKeyAfterRemap); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION

    return false;
  }

  bool
  RemapClass::Item::remap_SimultaneousKeyPresses(void)
  {
    if (isblocked()) return false;

#define CALL_UNION_FUNCTION(POINTER) {          \
    if (POINTER) { return (POINTER)->remap(); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES: CALL_UNION_FUNCTION(p_.simultaneousKeyPresses); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION

    return false;
  }

  bool
  RemapClass::Item::remap_setkeyboardtype(KeyboardType& keyboardType)
  {
    if (isblocked()) return false;

#define CALL_UNION_FUNCTION(POINTER) {                      \
    if (POINTER) { return (POINTER)->remap(keyboardType); } \
}

    switch (type_) {
      case BRIDGE_REMAPTYPE_SETKEYBOARDTYPE: CALL_UNION_FUNCTION(p_.setKeyboardType); break;
      default:
        // do nothing. (Do not call IOLOG_ERROR)
        break;
    }

#undef CALL_UNION_FUNCTION

    return false;
  }

  bool
  RemapClass::Item::isblocked(void)
  {
    if (! filters_) return false;

    for (size_t i = 0; i < filters_->size(); ++i) {
      RemapFilter::FilterUnion* p = (*filters_)[i];
      if (p && p->isblocked()) return true;
    }

    return false;
  }

  // ----------------------------------------------------------------------
  int RemapClass::allocation_count_ = 0;

  RemapClass::RemapClass(const unsigned int* initialize_vector, bool enabledvalue) :
    statusmessage_(NULL),
    enabled_(enabledvalue),
    is_simultaneouskeypresses_(false)
  {
    // ------------------------------------------------------------
    // check parameters.
    //
#define CHECK_INITIALIZE_VECTOR {                                 \
    if (! initialize_vector) {                                    \
      IOLOG_ERROR("RemapClass::RemapClass invalid parameter.\n"); \
      return;                                                     \
    }                                                             \
}

    CHECK_INITIALIZE_VECTOR;

    // --------------------
    // check version
    unsigned int version = initialize_vector[0];
    ++initialize_vector;
    CHECK_INITIALIZE_VECTOR;

    if (version != BRIDGE_REMAPCLASS_INITIALIZE_VECTOR_FORMAT_VERSION) {
      IOLOG_INFO("RemapClass::RemapClass version mismatch.\n");
      return;
    }

    // ------------------------------------------------------------
    // initialize items_ from vector
    unsigned int total_length = initialize_vector[0];
    ++initialize_vector;
    CHECK_INITIALIZE_VECTOR;

    if (total_length == 0) return;

    if (allocation_count_ + total_length > MAX_ALLOCATION_COUNT) {
      IOLOG_ERROR("RemapClass::RemapClass too many allocation_count_.\n");
      return;
    }
    allocation_count_ += total_length;

    // --------------------
    unsigned int total_tmp = 0;
    for (;;) {
      unsigned int size = initialize_vector[0];
      ++initialize_vector;
      ++total_tmp;
      CHECK_INITIALIZE_VECTOR;

      // ----------------------------------------
      if (size > 0) {
        unsigned int type = initialize_vector[0];

        if (BRIDGE_REMAPTYPE_NONE < type && type < BRIDGE_REMAPTYPE_END) {
          Item* newp = new Item(initialize_vector, size);
          items_.push_back(newp);

          if (type == BRIDGE_REMAPTYPE_SIMULTANEOUSKEYPRESSES) {
            is_simultaneouskeypresses_ = true;
          }

        } else if (BRIDGE_FILTERTYPE_NONE < type && type < BRIDGE_FILTERTYPE_END) {
          if (items_.size() == 0) {
            IOLOG_ERROR("RemapClass::RemapClass invalid filter (%d).\n", type);
            return;
          }
          Item* p = items_.back();
          if (p) {
            p->append_filter(initialize_vector, size);
          }

        } else if (type == BRIDGE_STATUSMESSAGE) {
          if (statusmessage_) {
            delete[] statusmessage_;
          }
          statusmessage_ = new char[size];
          if (statusmessage_) {
            for (size_t i = 0; i < size - 1; ++i) {
              statusmessage_[i] = initialize_vector[i + 1];
            }
            statusmessage_[size - 1] = '\0';
          }

        } else if (type == BRIDGE_VK_CONFIG) {
          if (size == 5) {
            unsigned int keycode_toggle         = initialize_vector[1];
            unsigned int keycode_force_on       = initialize_vector[2];
            unsigned int keycode_force_off      = initialize_vector[3];
            unsigned int keycode_sync_keydownup = initialize_vector[4];
            Handle_VK_CONFIG::add_item(this,
                                       keycode_toggle,
                                       keycode_force_on,
                                       keycode_force_off,
                                       keycode_sync_keydownup);
          }

        } else {
          IOLOG_ERROR("RemapClass::RemapClass unknown type:%d.\n", type);
          return;
        }

        initialize_vector += size;
        total_tmp += size;
        CHECK_INITIALIZE_VECTOR;
      }

      // ----------------------------------------
      if (total_tmp == total_length) return;
      if (total_tmp > total_length) {
        IOLOG_ERROR("RemapClass::RemapClass invalid initialize_vector. (total_length:%d, total_tmp:%d)\n", total_length, total_tmp);
        return;
      }
    }
  }

  RemapClass::~RemapClass(void)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        delete p;
      }
    }
    if (statusmessage_) {
      delete[] statusmessage_;
    }
  }

  void
  RemapClass::remap_setkeyboardtype(KeyboardType& keyboardType)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        if (p->remap_setkeyboardtype(keyboardType)) return;
      }
    }
  }

  void
  RemapClass::remap_key(RemapParams& remapParams)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        if (p->remap(remapParams)) return;
      }
    }
  }

  void
  RemapClass::remap_consumer(RemapConsumerParams& remapParams)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        if (p->remap(remapParams)) return;
      }
    }
  }

  void
  RemapClass::remap_pointing(RemapPointingParams_relative& remapParams)
  {
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        if (p->remap(remapParams)) return;
      }
    }
  }

  bool
  RemapClass::remap_simultaneouskeypresses(void)
  {
    bool queue_changed = false;
    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        if (p->remap_SimultaneousKeyPresses()) {
          queue_changed = true;
        }
      }
    }

    return queue_changed;
  }

  bool
  RemapClass::remap_dropkeyafterremap(const Params_KeyboardEventCallBack& params)
  {
    if (! enabled()) return false;

    for (size_t i = 0; i < items_.size(); ++i) {
      Item* p = items_[i];
      if (p) {
        if (p->drop(params)) return true;
      }
    }

    return false;
  }

  const char*
  RemapClass::get_statusmessage(void)
  {
    return statusmessage_;
  }

  bool
  RemapClass::is_simultaneouskeypresses(void)
  {
    return is_simultaneouskeypresses_;
  }

  void
  RemapClass::log_allocation_count(void)
  {
    IOLOG_INFO("RemapClass::allocation_count_ %d/%d (capacity %d%%)\n", allocation_count_, MAX_ALLOCATION_COUNT, allocation_count_ * 100 / MAX_ALLOCATION_COUNT);
  }

  void
  RemapClass::reset_allocation_count(void)
  {
    allocation_count_ = 0;
  }

  // ================================================================================
  namespace RemapClassManager {
    IOLock* lock_ = NULL;
    TimerWrapper refresh_timer_;

    char statusmessage_[32];
    char lastmessage_[32];
    bool isEventInputQueueDelayEnabled_ = false;

    Vector_RemapClassPointer* remapclasses_ = NULL;
    Vector_RemapClassPointer* enabled_remapclasses_ = NULL;

    // ======================================================================
    static void
    refresh_core(OSObject* owner, IOTimerEventSource* sender)
    {
      IOLockWrapper::ScopedLock lk(lock_);

      if (! remapclasses_) return;

      // ----------------------------------------
      if (enabled_remapclasses_) {
        delete enabled_remapclasses_;
      }
      enabled_remapclasses_ = new Vector_RemapClassPointer();
      if (! enabled_remapclasses_) return;

      // ----------------------------------------
      KeyboardRepeat::cancel();

      statusmessage_[0] = '\0';

      isEventInputQueueDelayEnabled_ = false;

      for (size_t i = 0; i < remapclasses_->size(); ++i) {
        RemapClass* p = (*remapclasses_)[i];
        if (! p) continue;

        if (p->enabled()) {
          enabled_remapclasses_->push_back(p);

          const char* msg = p->get_statusmessage();
          if (msg) {
            strlcat(statusmessage_, msg, sizeof(statusmessage_));
            strlcat(statusmessage_, " ", sizeof(statusmessage_));
          }

          if (p->is_simultaneouskeypresses()) {
            isEventInputQueueDelayEnabled_ = true;
          }
        }
      }

      if (strcmp(statusmessage_, lastmessage_) != 0) {
        KeyRemap4MacBook_bridge::StatusMessage::Request request(KeyRemap4MacBook_bridge::StatusMessage::MESSAGETYPE_EXTRA, statusmessage_);
        KeyRemap4MacBook_client::sendmsg(KeyRemap4MacBook_bridge::REQUEST_STATUS_MESSAGE, &request, sizeof(request), NULL, 0);
        strlcpy(lastmessage_, statusmessage_, sizeof(lastmessage_));
      }
    }

    // ======================================================================

    void
    initialize(IOWorkLoop& workloop)
    {
      lock_ = IOLockWrapper::alloc();
      statusmessage_[0] = '\0';
      lastmessage_[0] = '\0';
      remapclasses_ = NULL;
      enabled_remapclasses_ = NULL;

      refresh_timer_.initialize(&workloop, NULL, refresh_core);
    }

    void
    clear_xml(void)
    {
      Handle_VK_CONFIG::clear_items();

      if (enabled_remapclasses_) {
        delete enabled_remapclasses_;
        enabled_remapclasses_ = NULL;
      }

      if (remapclasses_) {
        for (size_t i = 0; i < remapclasses_->size(); ++i) {
          RemapClass* p = (*remapclasses_)[i];
          if (p) {
            delete p;
          }
        }
        delete remapclasses_;
        remapclasses_ = NULL;
      }
    }

    void
    terminate(void)
    {
      refresh_timer_.terminate();

      clear_xml();

      IOLockWrapper::free(lock_);
    }

    bool
    reload_xml(void)
    {
      IOLockWrapper::ScopedLock lk(lock_);

      bool retval = false;
      uint32_t count = 0;
      KeyRemap4MacBook_bridge::GetConfigInfo::Reply::Item* configinfo = NULL;

      // ------------------------------------------------------------
      if (Config::do_reload_only_config != 1) {
        clear_xml();
        remapclasses_ = new Vector_RemapClassPointer();
      }

      if (! remapclasses_) {
        IOLOG_ERROR("do_reload_xml remapclasses_ == NULL.\n");
        goto finish;
      }

      // ------------------------------------------------------------
      // get count
      {
        KeyRemap4MacBook_bridge::GetConfigCount::Reply reply;
        time_t timeout_second = 3;
        int error = KeyRemap4MacBook_client::sendmsg(KeyRemap4MacBook_bridge::REQUEST_GET_CONFIG_COUNT, NULL, 0, &reply, sizeof(reply), timeout_second, 0);
        if (error) {
          IOLOG_ERROR("do_reload_xml GetConfigCount sendmsg failed. (%d)\n", error);
          goto finish;
        }
        count = reply.count;
      }

      if (count > RemapClass::MAX_CONFIG_COUNT) {
        IOLOG_ERROR("do_reload_xml too many config count. (%d)\n", count);
        goto finish;
      }

      // ------------------------------------------------------------
      // get configinfo
      configinfo = new KeyRemap4MacBook_bridge::GetConfigInfo::Reply::Item[count];
      if (! configinfo) {
        IOLOG_ERROR("do_reload_xml allocation failed.\n");
        goto finish;
      }

      {
        KeyRemap4MacBook_bridge::GetConfigInfo::Reply* reply = reinterpret_cast<KeyRemap4MacBook_bridge::GetConfigInfo::Reply*>(configinfo);
        time_t timeout_second = 3;
        int error = KeyRemap4MacBook_client::sendmsg(KeyRemap4MacBook_bridge::REQUEST_GET_CONFIG_INFO,
                                                     NULL, 0,
                                                     reply, static_cast<uint32_t>(sizeof(configinfo[0]) * count),
                                                     timeout_second, 0);
        if (error) {
          IOLOG_ERROR("do_reload_xml GetConfigInfo sendmsg failed. (%d)\n", error);
          goto finish;
        }
      }

      // ------------------------------------------------------------
      if (Config::do_reload_only_config == 1) {
        if (count != remapclasses_->size()) {
          IOLOG_ERROR("do_reload_xml count != remapclasses_->size()\n");
          goto finish;
        }

        for (uint32_t i = 0; i < count; ++i) {
          (*remapclasses_)[i]->setEnabled(configinfo[i].enabled);
        }

      } else {
        remapclasses_->reserve(count);
        RemapClass::reset_allocation_count();

        // get initialize_vector
        for (uint32_t i = 0; i < count; ++i) {
          uint32_t size = configinfo[i].initialize_vector_size;
          uint32_t* initialize_vector = NULL;

          if (size > RemapClass::MAX_INITIALIZE_VECTOR_SIZE) {
            IOLOG_ERROR("do_reload_xml too large initialize_vector. (%d)\n", size);
            goto finish;
          }

          initialize_vector = new uint32_t[size];
          {
            KeyRemap4MacBook_bridge::GetConfigInitializeVector::Request request(i);
            KeyRemap4MacBook_bridge::GetConfigInitializeVector::Reply* reply = reinterpret_cast<KeyRemap4MacBook_bridge::GetConfigInitializeVector::Reply*>(initialize_vector);
            time_t timeout_second = 3;
            int error = KeyRemap4MacBook_client::sendmsg(KeyRemap4MacBook_bridge::REQUEST_GET_CONFIG_INITIALIZE_VECTOR,
                                                         &request, sizeof(request),
                                                         reply, static_cast<uint32_t>(sizeof(initialize_vector[0]) * size),
                                                         timeout_second, 0);
            RemapClass* newp = NULL;
            if (! error) {
              newp = new RemapClass(initialize_vector, configinfo[i].enabled);
            }
            remapclasses_->push_back(newp);
          }
          delete[] initialize_vector;
        }

        RemapClass::log_allocation_count();
      }

      refresh();

      retval = true;

    finish:
      if (configinfo) {
        delete[] configinfo;
      }
      return retval;
    }

    void
    refresh(void)
    {
      // We use timer to prevent deadlock of lock_. (refresh may be called in the remap_key, remap_consumer, *).
      enum {
        DELAY = 1,
      };
      refresh_timer_.setTimeoutMS(DELAY);
    }

    // ----------------------------------------------------------------------
#define CALL_REMAPCLASS_FUNC(FUNC, PARAMS) {                       \
    IOLockWrapper::ScopedLock lk(lock_);                           \
    if (enabled_remapclasses_) {                                   \
      for (size_t i = 0; i < enabled_remapclasses_->size(); ++i) { \
        RemapClass* p = (*enabled_remapclasses_)[i];               \
        if (p) p->FUNC(PARAMS);                                    \
      }                                                            \
    }                                                              \
}

    void
    remap_setkeyboardtype(KeyboardType& keyboardType)
    {
      CALL_REMAPCLASS_FUNC(remap_setkeyboardtype, keyboardType);
    }

    void
    remap_key(RemapParams& remapParams)
    {
      CALL_REMAPCLASS_FUNC(remap_key, remapParams);
    }

    void
    remap_consumer(RemapConsumerParams& remapParams)
    {
      CALL_REMAPCLASS_FUNC(remap_consumer, remapParams);
    }

    void
    remap_pointing(RemapPointingParams_relative& remapParams)
    {
      CALL_REMAPCLASS_FUNC(remap_pointing, remapParams);
    }

#undef CALL_REMAPCLASS_FUNC

    bool
    remap_simultaneouskeypresses(void)
    {
      IOLockWrapper::ScopedLock lk(lock_);

      bool queue_changed = false;

      if (enabled_remapclasses_) {
        for (size_t i = 0; i < enabled_remapclasses_->size(); ++i) {
          RemapClass* p = (*enabled_remapclasses_)[i];
          if (p) {
            if (p->remap_simultaneouskeypresses()) {
              queue_changed = true;
            }
          }
        }
      }

      return queue_changed;
    }

    bool
    remap_dropkeyafterremap(const Params_KeyboardEventCallBack& params)
    {
      IOLockWrapper::ScopedLock lk(lock_);

      bool dropped = false;

      if (enabled_remapclasses_) {
        for (size_t i = 0; i < enabled_remapclasses_->size(); ++i) {
          RemapClass* p = (*enabled_remapclasses_)[i];
          if (p) {
            if (p->remap_dropkeyafterremap(params)) dropped = true;
          }
        }
      }

      return dropped;
    }

    bool
    isEventInputQueueDelayEnabled(void)
    {
      return isEventInputQueueDelayEnabled_;
    }

    bool
    isEnabled(size_t configindex)
    {
      // isEnabled called in remap_key, remap_consumer, ...
      // So, don't make ScopedLock(lock_).

      if (! remapclasses_) return false;

      if (configindex >= remapclasses_->size()) {
        IOLOG_ERROR("RemapClass::isEnabled invalid configindex.\n");
        return false;
      }

      RemapClass* p = (*remapclasses_)[configindex];
      if (! p) return false;

      return p->enabled();
    }
  }
}
