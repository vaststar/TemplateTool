#pragma once

#include <ucf/Utilities/LoggerUtils/LoggerExport.h>

static inline const char* ContactListViewModelLoggerName = ucf::utilities::kAppLoggerName;
static inline const char* ContactListViewModelLoggerTag = "ContactListViewModel";

#define CONTACT_LIST_VIEW_MODEL_LOG_DEBUG(message) UCF_LOG_DEBUG(ContactListViewModelLoggerTag, message, ContactListViewModelLoggerName)
#define CONTACT_LIST_VIEW_MODEL_LOG_INFO(message) UCF_LOG_INFO(ContactListViewModelLoggerTag, message, ContactListViewModelLoggerName)
#define CONTACT_LIST_VIEW_MODEL_LOG_WARN(message) UCF_LOG_WARN(ContactListViewModelLoggerTag, message, ContactListViewModelLoggerName)
#define CONTACT_LIST_VIEW_MODEL_LOG_ERROR(message) UCF_LOG_ERROR(ContactListViewModelLoggerTag, message, ContactListViewModelLoggerName)
#define CONTACT_LIST_VIEW_MODEL_LOG_FATAL(message) UCF_LOG_FATAL(ContactListViewModelLoggerTag, message, ContactListViewModelLoggerName)
