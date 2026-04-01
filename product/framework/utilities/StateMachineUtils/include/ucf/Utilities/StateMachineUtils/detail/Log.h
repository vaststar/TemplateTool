#pragma once

/// @file detail/Log.h
/// @brief Pluggable logging macros for the FSM library.
///
/// By default all macros expand to no-ops (zero overhead).
///
/// To enable logging, define FSM_LOG_HEADER *before* including StateMachine.h:
///
///   #define FSM_LOG_HEADER "MyFSMLoggerDefine.h"
///   #include <ucf/Utilities/StateMachineUtils/StateMachine.h>
///
/// The header file referenced by FSM_LOG_HEADER should define:
///   FSM_LOG_DEBUG(msg)
///   FSM_LOG_INFO(msg)
///   FSM_LOG_WARN(msg)
///   FSM_LOG_ERROR(msg)
///
/// where `msg` supports the ostream << style, e.g.:
///   FSM_LOG_INFO("transition: " << fromName << " -> " << toName)
///
/// Example MyFSMLoggerDefine.h (integrating with UCF logger):
///
///   #pragma once
///   #include <ucf/Utilities/LoggerUtils/LoggerExport.h>
///
///   static inline const char* FSMLoggerName = ucf::utilities::kAppLoggerName;
///   static inline const char* FSMLoggerTag  = "MyModuleFSM";
///
///   #define FSM_LOG_DEBUG(msg) UCF_LOG_DEBUG(FSMLoggerTag, msg, FSMLoggerName)
///   #define FSM_LOG_INFO(msg)  UCF_LOG_INFO(FSMLoggerTag, msg, FSMLoggerName)
///   #define FSM_LOG_WARN(msg)  UCF_LOG_WARN(FSMLoggerTag, msg, FSMLoggerName)
///   #define FSM_LOG_ERROR(msg) UCF_LOG_ERROR(FSMLoggerTag, msg, FSMLoggerName)

#ifdef FSM_LOG_HEADER
#  include FSM_LOG_HEADER
#endif

#ifndef FSM_LOG_DEBUG
#  define FSM_LOG_DEBUG(msg) (void)0
#endif

#ifndef FSM_LOG_INFO
#  define FSM_LOG_INFO(msg) (void)0
#endif

#ifndef FSM_LOG_WARN
#  define FSM_LOG_WARN(msg) (void)0
#endif

#ifndef FSM_LOG_ERROR
#  define FSM_LOG_ERROR(msg) (void)0
#endif
