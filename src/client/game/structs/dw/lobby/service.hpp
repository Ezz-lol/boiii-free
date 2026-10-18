#pragma once

#include "core.hpp"

#include <game/structs/dw/core.hpp>
#include <game/structs/dw/crypt.hpp>
#include <game/structs/dw/hash.hpp>
#include <game/structs/dw/net/resolution.hpp>
#include <game/structs/dw/task/core.hpp>

namespace game {
namespace dw {

namespace publisher {
namespace variables {
class bdPublisherVariables;
}
} // namespace publisher
namespace reward {
class bdReward;
}

namespace storage {
class bdStorage;
}

namespace matchmaking {
class bdMatchMaking;
namespace queued {
class bdQueuedMatching;
}
} // namespace matchmaking

namespace stats {
class bdStats;
}

namespace content {
namespace unlock {
class bdContentUnlock;
}
namespace streaming {
class bdContentStreaming;
class bdPooledStorage;
} // namespace streaming
} // namespace content

namespace title {
class bdTitleUtilities;
}

namespace event {
namespace log {
class bdEventLog;
}
} // namespace event

namespace key {
namespace archive {
class bdKeyArchive;
}
} // namespace key

namespace counter {
class bdCounter;
}

namespace tag {
class bdTags;
}

namespace vote {
namespace rank {
class bdVoteRank;
}
class bdVoting;
} // namespace vote

namespace social {

namespace external {
namespace twitch {
class bdTwitch;
}
namespace youtube {
class bdYouTube;
}
namespace twitter {
class bdTwitter;
}
namespace facebook {
class bdFacebook;
}
} // namespace external

namespace profile {
class bdProfiles;
}

namespace messaging {
class bdMessaging;
}

namespace group {
class bdGroup;
}

namespace friends {
class bdFriends;
}

namespace league {
class bdLeague;
}

namespace teams {
class bdTeams;
}

namespace mail {
class bdMail;
}
} // namespace social

namespace linkcode {
class bdLinkCode;
}

namespace anticheat {
class bdAntiCheat;
}

namespace dml {
class bdDML;
}

namespace identity {
namespace umbrella {
class bdUmbrella;
}
namespace uno {
class bdUno;
}
} // namespace identity

namespace user {
namespace groups {
class bdUserGroups;
}
namespace blocklist {
class bdUserBlockList;
}
} // namespace user

namespace presence {
namespace rich {
class bdRichPresenceService;
}
class bdPresenceService;
} // namespace presence

namespace commerce {
class bdCommerce;

namespace marketing {
class bdMarketingComms;
}

namespace marketplace {
class bdMarketplace;
class bdTeamMarketplace;
} // namespace marketplace
} // namespace commerce

namespace subscription {
class bdSubscription;
}

namespace featureban {
class bdFeatureBan;
}

namespace tencent {
class bdTencent;
}

namespace codonlineteams {
class bdCodOnlineTeams;
}

namespace relay {
class bdRelayService;
}

namespace linkedaccounts {
class bdLinkedAccounts;
}

namespace lobby {
namespace service {

class __declspec(align(8)) bdLobbyService : bdLobbyConnectionListener {
public:
  task::manage::bdRemoteTaskManager *m_taskManager;
  social::profile::bdProfiles *m_profiles;
  social::messaging::bdMessaging *m_messaging;
  matchmaking::bdMatchMaking *m_matchmaking;
  stats::bdStats *m_statsManager;
  social::friends::bdFriends *m_friendsManager;
  content::unlock::bdContentUnlock *m_contentUnlockManager;
  title::bdTitleUtilities *m_titleUtilitiesManager;
  event::log::bdEventLog *m_eventLogManager;
  key::archive::bdKeyArchive *m_keyArchive;
  counter::bdCounter *m_counter;
  social::group::bdGroup *m_group;
  content::streaming::bdContentStreaming *m_contentStreamingManager;
  content::streaming::bdPooledStorage *m_pooledStorageManager;
  tag::bdTags *m_tags;
  vote::rank::bdVoteRank *m_voteRankManager;
  social::external::twitch::bdTwitch *m_twitch;
  social::external::youtube::bdYouTube *m_youTube;
  social::external::twitter::bdTwitter *m_twitter;
  social::external::facebook::bdFacebook *m_facebook;
  linkcode::bdLinkCode *m_linkCode;
  anticheat::bdAntiCheat *m_antiCheat;
  dml::bdDML *m_DML;
  identity::umbrella::bdUmbrella *m_umbrella;
  identity::uno::bdUno *m_uno;
  user::groups::bdUserGroups *m_userGroups;
  presence::rich::bdRichPresenceService *m_richPresence;
  presence::bdPresenceService *m_presence;
  commerce::bdCommerce *m_commerce;
  subscription::bdSubscription *m_subscription;
  featureban::bdFeatureBan *m_featureBan;
  tencent::bdTencent *m_tencent;
  codonlineteams::bdCodOnlineTeams *m_codOnlineTeams;
  social::league::bdLeague *m_league;
  relay::bdRelayService *m_relayService;
  linkedaccounts::bdLinkedAccounts *m_linkedAccounts;
  matchmaking::queued::bdQueuedMatching *m_queuedMatching;
  vote::bdVoting *m_voting;
  user::blocklist::bdUserBlockList *m_userBlockList;
  hash::bdHashMap<bdString, commerce::marketing::bdMarketingComms *>
      m_marketingCommsManagers;
  hash::bdHashMap<bdString, commerce::marketplace::bdMarketplace *>
      m_marketplaceManagers;
  hash::bdHashMap<bdString, commerce::marketplace::bdTeamMarketplace *>
      m_teamMarketplaceManagers;
  hash::bdHashMap<bdString, reward::bdReward *> m_rewardManagers;
  reward::bdReward *m_rewards;
  hash::bdHashMap<bdString, publisher::variables::bdPublisherVariables *>
      m_publisherVariablesManagers;
  hash::bdHashMap<bdString, storage::bdStorage *> m_storageManagers;
  hash::bdHashMap<bdString, social::teams::bdTeams *> m_teamsManagers;
  hash::bdHashMap<bdString, social::mail::bdMail *> m_mailManagers;
  hash::bdHashMap<bdString, linkcode::bdLinkCode *> m_linkCodeManagers;
  net::resolution::bdGetHostByName m_LSGLookup;
  bdNChar8 *m_LSGAddress;
  bdPort m_LSGPort;
  bdAddr m_resolvedLSGAddress;
  bdBool m_isResolved;
  bdTitleID m_titleID;
  bdLobbyEventHandler *m_eventHandler;
  bdLobbyConnectionRef m_lobbyConnection;
  bdBool m_lobbyConnectionEstablished;
  bdAuthInfo m_authInfo;
  bdBool m_encryptedConnection;
  bdLobbyErrorCode m_errorCode;
  bdUInt32 m_lastPumpTime;
  bdBool m_markTasksThrottled;
  bdBool m_uploadThrottled;
  bdUInt32 m_uploadThrottleBytesPerMs;
  bdUInt32 m_uploadThrottleBucketSize;
};

} // namespace service
} // namespace lobby
} // namespace dw
} // namespace game
