#pragma once

#include <game/structs/core.hpp>

#include <steam/steam.hpp>

namespace game {

bool is_server();
bool is_client();

namespace steam {

#include <steam/steamtypes.h> // IWYU pragma: export

#include <steam/isteamremotestorage.h> // IWYU pragma: export
#include <steam/isteamugc.h>           // IWYU pragma: export
#include <steam/isteamuser.h>          // IWYU pragma: export

#include <steam/isteamapps.h>    // IWYU pragma: export
#include <steam/isteamfriends.h> // IWYU pragma: export

struct SteamInterfacesBase {
  ISteamClient *pSteamClient;
};

struct SteamInterfaces;

namespace cl {
// Steam interfaces struct on client engine
struct SteamInterfaces : SteamInterfacesBase {
  ISteamUser *pSteamUser;
  ISteamFriends *pSteamFriends;
  ISteamUtils *pSteamUtils;
  ISteamMatchmaking *pSteamMatchmaking;
  ISteamUserStats *pSteamUserStats;
  ISteamApps *pSteamApps;
  ISteamMatchmakingServers *pSteamMatchmakingServers;
  ISteamNetworking *pSteamNetworking;
  ISteamRemoteStorage *pSteamRemoteStorage;
  ISteamScreenshots *pSteamScreenshots;
  ISteamHTTP *pSteamHTTP;
  ISteamUnifiedMessages *pSteamUnifiedMessages;
  ISteamController *pSteamController;
  ISteamUGC *pSteamUGC;
  ISteamAppList *pSteamAppList;
  ISteamMusic *pSteamMusic;
  ISteamMusicRemote *pSteamMusicRemote;
  ISteamHTMLSurface *pSteamHTMLSurface;
  ISteamInventory *pSteamInventory;
  ISteamVideo *pSteamVideo;

  constexpr operator game::steam::SteamInterfaces() noexcept;
};
} // namespace cl
namespace sv {
// Steam interfaces struct on server engine
struct SteamInterfaces : SteamInterfacesBase {
  ISteamGameServer *pSteamGameServer;
  ISteamUtils *pSteamUtils;
  ISteamNetworking *pSteamNetworking;
  ISteamGameServerStats *pSteamGameServerStats;
  ISteamHTTP *pSteamHTTP;
  ISteamInventory *pSteamInventory;
  ISteamUGC *pSteamUGC;
  ISteamApps *pSteamApps;

  constexpr operator game::steam::SteamInterfaces() noexcept;
};
} // namespace sv

struct SteamInterfaces
    : public EngineDependentMut<cl::SteamInterfaces, sv::SteamInterfaces> {

  inline ISteamClient *pSteamClient() {
    if (game::is_server()) {
      return sv->pSteamClient;
    }
    return cl->pSteamClient;
  }

  inline ISteamUtils *pSteamUtils() {
    if (game::is_server()) {
      return sv->pSteamUtils;
    }
    return cl->pSteamUtils;
  }

  inline ISteamApps *pSteamApps() {
    if (game::is_server()) {
      return sv->pSteamApps;
    }
    return cl->pSteamApps;
  }

  inline ISteamNetworking *pSteamNetworking() {
    if (game::is_server()) {
      return sv->pSteamNetworking;
    }
    return cl->pSteamNetworking;
  }

  inline ISteamHTTP *pSteamHTTP() {
    if (game::is_server()) {
      return sv->pSteamHTTP;
    }
    return cl->pSteamHTTP;
  }

  inline ISteamUGC *pSteamUGC() {
    if (game::is_server()) {
      return sv->pSteamUGC;
    }
    return cl->pSteamUGC;
  }

  inline ISteamInventory *pSteamInventory() {
    if (game::is_server()) {
      return sv->pSteamInventory;
    }
    return cl->pSteamInventory;
  }

  // Function-based constructors to allow usage without violating CPP 2003 PoD
  // conformance
  static inline constexpr SteamInterfaces
  from(sv::SteamInterfaces *s) noexcept {
    /*
      Cannot access anonymous union members in inherited struct when used in
      inline `{.anonMember}` syntax, so we have to declare and define the struct
      separately.
    */
    SteamInterfaces result{};
    result.sv = s;
    return result;
  }

  static inline constexpr SteamInterfaces
  from(cl::SteamInterfaces *c) noexcept {
    SteamInterfaces result{};
    result.cl = c;
    return result;
  }

  static inline constexpr SteamInterfaces from(void *v) noexcept {
    SteamInterfaces result{};
    result.ptr = v;
    return result;
  }
};
ASSERT_CPP03_POD(SteamInterfaces);

constexpr cl::SteamInterfaces::operator game::steam::
    SteamInterfaces() noexcept {
  return game::steam::SteamInterfaces::from(this);
}

constexpr sv::SteamInterfaces::operator game::steam::
    SteamInterfaces() noexcept {
  return game::steam::SteamInterfaces::from(this);
}

/*
   Including steam_api.h here causes conflicts with some of our steam_proxy
   types, classes, and defines, so it cannot be included.

   The following classes are only defined in steam_api.h, so their definitions
   were moved here for usage.
*/

//-----------------------------------------------------------------------------
// Purpose: base for callbacks and call results - internal implementation detail
//-----------------------------------------------------------------------------

class CCallbackBase {
public:
  CCallbackBase() {
    m_nCallbackFlags = 0;
    m_iCallback = 0;
  }
  // don't add a virtual destructor because we export this binary interface
  // across dll's
  virtual void Run(void *pvParam) = 0;
  virtual void Run(void *pvParam, bool bIOFailure,
                   SteamAPICall_t hSteamAPICall) = 0;
  int GetICallback() { return m_iCallback; }
  virtual int GetCallbackSizeBytes() = 0;

protected:
  enum { k_ECallbackFlagsRegistered = 0x01, k_ECallbackFlagsGameServer = 0x02 };
  uint8 m_nCallbackFlags;
  int m_iCallback;
  friend class CCallbackMgr;

private:
  CCallbackBase(const CCallbackBase &);
  CCallbackBase &operator=(const CCallbackBase &);
};

//-----------------------------------------------------------------------------
// Purpose: maps a steam async call result to a class member function
//			template params: T = local class, P = parameter struct
//-----------------------------------------------------------------------------
template <class T, class P> class CCallResult : private CCallbackBase {
public:
  typedef void (T::*func_t)(P *, bool);

  CCallResult();
  ~CCallResult();

  void Set(SteamAPICall_t hAPICall, T *p, func_t func);
  bool IsActive() const;
  void Cancel();

  void SetGameserverFlag() { m_nCallbackFlags |= k_ECallbackFlagsGameServer; }

private:
  virtual void Run(void *pvParam);
  virtual void Run(void *pvParam, bool bIOFailure,
                   SteamAPICall_t hSteamAPICall);
  virtual int GetCallbackSizeBytes() { return sizeof(P); }

  SteamAPICall_t m_hAPICall;
  T *m_pObj;
  func_t m_Func;
};

#define _STEAM_CALLBACK_AUTO_HOOK(thisclass, func, param)
#define _STEAM_CALLBACK_HELPER(_1, _2, SELECTED, ...) _STEAM_CALLBACK_##SELECTED
#define _STEAM_CALLBACK_SELECT(X, Y) _STEAM_CALLBACK_HELPER X Y
#define _STEAM_CALLBACK_3(extra_code, thisclass, func, param)                  \
  struct CCallbackInternal_##func : private CCallbackImpl<sizeof(param)> {     \
    CCallbackInternal_##func() {                                               \
      extra_code SteamAPI_RegisterCallback(this, param::k_iCallback);          \
    }                                                                          \
    CCallbackInternal_##func(const CCallbackInternal_##func &) {               \
      extra_code SteamAPI_RegisterCallback(this, param::k_iCallback);          \
    }                                                                          \
    CCallbackInternal_##func &operator=(const CCallbackInternal_##func &) {    \
      return *this;                                                            \
    }                                                                          \
                                                                               \
  private:                                                                     \
    virtual void Run(void *pvParam) {                                          \
      _STEAM_CALLBACK_AUTO_HOOK(thisclass, func, param)                        \
      thisclass *pOuter = reinterpret_cast<thisclass *>(                       \
          reinterpret_cast<char *>(this) -                                     \
          offsetof(thisclass, m_steamcallback_##func));                        \
      pOuter->func(reinterpret_cast<param *>(pvParam));                        \
    }                                                                          \
  } m_steamcallback_##func;                                                    \
  void func(param *pParam)
#define _STEAM_CALLBACK_4(_, thisclass, func, param, var)                      \
  CCallback<thisclass, param> var;                                             \
  void func(param *pParam)

//-----------------------------------------------------------------------------
// Purpose: maps a steam async call result to a class member function
//			template params: T = local class, P = parameter struct
//-----------------------------------------------------------------------------
template <class T, class P> inline CCallResult<T, P>::CCallResult() {
  m_hAPICall = k_uAPICallInvalid;
  m_pObj = nullptr;
  m_Func = nullptr;
  m_iCallback = P::k_iCallback;
}

template <class T, class P>
inline void CCallResult<T, P>::Set(SteamAPICall_t hAPICall, T *p, func_t func) {
  if (m_hAPICall)
    ::steam::SteamAPI_UnregisterCallResult(this, m_hAPICall);

  m_hAPICall = hAPICall;
  m_pObj = p;
  m_Func = func;

  if (hAPICall)
    ::steam::SteamAPI_RegisterCallResult(this, hAPICall);
}

template <class T, class P> inline bool CCallResult<T, P>::IsActive() const {
  return (m_hAPICall != k_uAPICallInvalid);
}

template <class T, class P> inline void CCallResult<T, P>::Cancel() {
  if (m_hAPICall != k_uAPICallInvalid) {
    ::steam::SteamAPI_UnregisterCallResult(
        reinterpret_cast<::steam::callbacks::base *>(this), m_hAPICall);
    m_hAPICall = k_uAPICallInvalid;
  }
}

template <class T, class P> inline CCallResult<T, P>::~CCallResult() {
  Cancel();
}

template <class T, class P> inline void CCallResult<T, P>::Run(void *pvParam) {
  m_hAPICall = k_uAPICallInvalid; // caller unregisters for us
  (m_pObj->*m_Func)((P *)pvParam, false);
}

template <class T, class P>
inline void CCallResult<T, P>::Run(void *pvParam, bool bIOFailure,
                                   SteamAPICall_t hSteamAPICall) {
  if (hSteamAPICall == m_hAPICall) {
    m_hAPICall = k_uAPICallInvalid; // caller unregisters for us
    (m_pObj->*m_Func)((P *)pvParam, bIOFailure);
  }
}

template <int sizeof_P> class CCallbackImpl : protected CCallbackBase {
public:
  ~CCallbackImpl() {
    if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
      ::steam::SteamAPI_UnregisterCallback(
          reinterpret_cast<::steam::callbacks::base *>(this));
  }
  void SetGameserverFlag() { m_nCallbackFlags |= k_ECallbackFlagsGameServer; }

protected:
  virtual void Run(void *pvParam) = 0;
  virtual void Run(void *pvParam, bool /*bIOFailure*/,
                   SteamAPICall_t /*hSteamAPICall*/) {
    Run(pvParam);
  }
  virtual int GetCallbackSizeBytes() { return sizeof_P; }
};

template <class T, class P, bool bGameserver = false>
class CCallback : public CCallbackImpl<sizeof(P)> {
public:
  typedef void (T::*func_t)(P *);

  // NOTE: If you can't provide the correct parameters at construction time, you
  // should use the CCallbackManual callback object (STEAM_CALLBACK_MANUAL
  // macro) instead.
  CCallback(T *pObj, func_t func);

  void Register(T *pObj, func_t func);
  void Unregister();

protected:
  virtual void Run(void *pvParam);

  T *m_pObj;
  func_t m_Func;
};

//-----------------------------------------------------------------------------
// Purpose: maps a steam callback to a class member function
//			template params: T = local class, P = parameter struct,
//			bGameserver = listen for gameserver callbacks instead of client
// callbacks
//-----------------------------------------------------------------------------
template <class T, class P, bool bGameserver>
inline CCallback<T, P, bGameserver>::CCallback(T *pObj, func_t func)
    : m_pObj(nullptr), m_Func(nullptr) {
  if (bGameserver) {
    this->SetGameserverFlag();
  }
  Register(pObj, func);
}

template <class T, class P, bool bGameserver>
inline void CCallback<T, P, bGameserver>::Register(T *pObj, func_t func) {
  if (!pObj || !func)
    return;

  if (this->m_nCallbackFlags & CCallbackBase::k_ECallbackFlagsRegistered)
    Unregister();

  m_pObj = pObj;
  m_Func = func;
  // SteamAPI_RegisterCallback sets k_ECallbackFlagsRegistered
  ::steam::SteamAPI_RegisterCallback(this, P::k_iCallback);
}

template <class T, class P, bool bGameserver>
inline void CCallback<T, P, bGameserver>::Unregister() {
  // SteamAPI_UnregisterCallback removes k_ECallbackFlagsRegistered
  ::steam::SteamAPI_UnregisterCallback(this);
}

template <class T, class P, bool bGameserver>
inline void CCallback<T, P, bGameserver>::Run(void *pvParam) {
  (m_pObj->*m_Func)((P *)pvParam);
}

} // namespace steam
} // namespace game
