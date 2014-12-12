LOCAL_PATH := $(call my-dir)/../../
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_CLANG := true

LOCAL_CFLAGS	:= -Wall \
-W \
-O2 \
-pipe \
-fPIC \
-frtti \
-fexceptions \
-D_ANDROID \

LOCAL_CPPFLAGS += -std=c++11

LOCAL_MODULE    := hfstack_android

LOCAL_EXPORT_C_INCLUDES:= $(LOCAL_PATH) \
		

LOCAL_C_INCLUDES:= $(LOCAL_PATH) \
$(LOCAL_PATH)/../easysqlite/easySQLite/easySQLite \
$(LOCAL_PATH)/openpeer/stack/internal \
$(LOCAL_PATH)/../ortc-lib/libs/zsLib \
$(LOCAL_PATH)/../ortc-lib/libs/zsLib/zsLib/internal \
$(LOCAL_PATH)/../ortc-lib/libs/zsLib/zsLib/extras \
$(LOCAL_PATH)/../ortc-lib/libs/op-services-cpp \
$(LOCAL_PATH)/../ortc-lib/libs \
$(LOCAL_PATH)/.. \
$(LOCAL_PATH)/../ortc-lib/libs/build/android/curl/include \
$(LOCAL_PATH)/../ortc-lib/libs/udns \
$(LOCAL_PATH)/../sqlite/build \
$(ANDROIDNDK_PATH)/sources/android/support/include \
$(ANDROIDNDK_PATH)/sources/cxx-stl/llvm-libc++/libcxx/include \
$(ANDROIDNDK_PATH)/platforms/android-19/arch-arm/usr/include \

SOURCE_PATH := openpeer/stack/cpp
MESSAGE_SOURCE_PATH := openpeer/stack/message

LOCAL_SRC_FILES := $(SOURCE_PATH)/stack.cpp \
		   $(SOURCE_PATH)/stack_Account.cpp \
		   $(SOURCE_PATH)/stack_AccountFinder.cpp \
		   $(SOURCE_PATH)/stack_AccountPeerLocation.cpp \
		   $(SOURCE_PATH)/stack_BootstrappedNetwork.cpp \
		   $(SOURCE_PATH)/stack_BootstrappedNetworkManager.cpp \
		   $(SOURCE_PATH)/stack_Cache.cpp \
		   $(SOURCE_PATH)/stack_Diff.cpp \
		   $(SOURCE_PATH)/stack_FinderConnection.cpp \
		   $(SOURCE_PATH)/stack_FinderRelayChannel.cpp \
		   $(SOURCE_PATH)/stack_Helper.cpp \
		   $(SOURCE_PATH)/stack_IServicePushMailboxSessionDatabaseAbstractionTables.cpp \
		   $(SOURCE_PATH)/stack_KeyGenerator.cpp \
		   $(SOURCE_PATH)/stack_Location.cpp \
		   $(SOURCE_PATH)/stack_MessageIncoming.cpp \
		   $(SOURCE_PATH)/stack_MessageMonitor.cpp \
		   $(SOURCE_PATH)/stack_MessageMonitorManager.cpp \
		   $(SOURCE_PATH)/stack_Peer.cpp \
		   $(SOURCE_PATH)/stack_PeerFilePrivate.cpp \
		   $(SOURCE_PATH)/stack_PeerFilePublic.cpp \
		   $(SOURCE_PATH)/stack_PeerFiles.cpp \
		   $(SOURCE_PATH)/stack_PeerSubscription.cpp \
		   $(SOURCE_PATH)/stack_Publication.cpp \
		   $(SOURCE_PATH)/stack_PublicationMetaData.cpp \
		   $(SOURCE_PATH)/stack_PublicationRepository.cpp \
		   $(SOURCE_PATH)/stack_PublicationRepository_Fetcher.cpp \
		   $(SOURCE_PATH)/stack_PublicationRepository_PeerCache.cpp \
		   $(SOURCE_PATH)/stack_PublicationRepository_PeerSubscriptionIncoming.cpp \
		   $(SOURCE_PATH)/stack_PublicationRepository_PeerSubscriptionOutgoing.cpp \
		   $(SOURCE_PATH)/stack_PublicationRepository_Publisher.cpp \
		   $(SOURCE_PATH)/stack_PublicationRepository_Remover.cpp \
		   $(SOURCE_PATH)/stack_PublicationRepository_SubscriptionLocal.cpp \
		   $(SOURCE_PATH)/stack_ServerMessaging.cpp \
		   $(SOURCE_PATH)/stack_ServiceCertificatesValidateQuery.cpp \
		   $(SOURCE_PATH)/stack_ServiceIdentitySession.cpp \
		   $(SOURCE_PATH)/stack_ServiceLockboxSession.cpp \
		   $(SOURCE_PATH)/stack_ServiceNamespaceGrantSession.cpp \
		   $(SOURCE_PATH)/stack_ServicePeerFileLookup.cpp \
		   $(SOURCE_PATH)/stack_ServicePushMailboxSession.cpp \
		   $(SOURCE_PATH)/stack_ServicePushMailboxSessionDatabaseAbstraction.cpp \
		   $(SOURCE_PATH)/stack_ServicePushMailboxSession_AsyncDecrypt.cpp \
		   $(SOURCE_PATH)/stack_ServicePushMailboxSession_AsyncEncrypt.cpp \
		   $(SOURCE_PATH)/stack_ServicePushMailboxSession_AsyncNotifier.cpp \
		   $(SOURCE_PATH)/stack_ServicePushMailboxSession_RegisterQuery.cpp \
		   $(SOURCE_PATH)/stack_ServicePushMailboxSession_SendQuery.cpp \
		   $(SOURCE_PATH)/stack_ServiceSaltFetchSignedSaltQuery.cpp \
		   $(SOURCE_PATH)/stack_Settings.cpp \
                   $(SOURCE_PATH)/stack_javascript.cpp \
		   $(SOURCE_PATH)/stack_stack.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_Message.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_MessageFactoryManager.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_MessageFactoryUnknown.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_MessageHelper.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_MessageNotify.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_MessageNotifyUnknown.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_MessageRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_MessageRequestUnknown.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_MessageResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/cpp/stack_message_messages.cpp \
		   $(MESSAGE_SOURCE_PATH)/bootstrapped-servers/cpp/ServersGetRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/bootstrapped-servers/cpp/ServersGetResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/bootstrapped-servers/cpp/MessageFactoryBootstrappedServers.cpp \
		   $(MESSAGE_SOURCE_PATH)/bootstrapper/cpp/MessageFactoryBootstrapper.cpp \
		   $(MESSAGE_SOURCE_PATH)/bootstrapper/cpp/ServicesGetRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/bootstrapper/cpp/ServicesGetResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/certificates/cpp/CertificatesGetRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/certificates/cpp/CertificatesGetResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/certificates/cpp/MessageFactoryCertificates.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityAccessCompleteNotify.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityAccessNamespaceGrantChallengeValidateRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityAccessNamespaceGrantChallengeValidateResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityAccessRolodexCredentialsGetRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityAccessRolodexCredentialsGetResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityAccessStartNotify.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityAccessWindowRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityAccessWindowResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityLookupUpdateRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/IdentityLookupUpdateResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity/cpp/MessageFactoryIdentity.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxAccessRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxAccessResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxContentGetRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxContentGetResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxContentSetRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxContentSetResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxIdentitiesUpdateRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxIdentitiesUpdateResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxNamespaceGrantChallengeValidateRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/LockboxNamespaceGrantChallengeValidateResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lockbox/cpp/MessageFactoryIdentityLockbox.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lookup/cpp/IdentityLookupCheckRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/identity-lookup/cpp/IdentityLookupCheckResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity-lookup/cpp/IdentityLookupRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity-lookup/cpp/IdentityLookupResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/identity-lookup/cpp/MessageFactoryIdentityLookup.cpp \
		  $(MESSAGE_SOURCE_PATH)/namespace-grant/cpp/MessageFactoryNamespaceGrant.cpp \
		  $(MESSAGE_SOURCE_PATH)/namespace-grant/cpp/NamespaceGrantCompleteNotify.cpp \
		  $(MESSAGE_SOURCE_PATH)/namespace-grant/cpp/NamespaceGrantStartNotify.cpp \
		  $(MESSAGE_SOURCE_PATH)/namespace-grant/cpp/NamespaceGrantWindowRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/namespace-grant/cpp/NamespaceGrantWindowResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/peer/cpp/MessageFactoryPeer.cpp \
		  $(MESSAGE_SOURCE_PATH)/peer/cpp/PeerServicesGetRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/peer/cpp/PeerServicesGetResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/peer/cpp/PeerFilesGetRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/peer/cpp/PeerFilesGetResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/peer/cpp/PeerFileSetRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/peer/cpp/PeerFileSetResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/MessageFactoryPeerCommon.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerDeleteRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerDeleteResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerGetRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerGetResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerPublishNotify.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerPublishRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerPublishResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerSubscribeRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-common/cpp/PeerSubscribeResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/ChannelMapNotify.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/ChannelMapRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/ChannelMapResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/MessageFactoryPeerFinder.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/PeerLocationFindNotify.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/PeerLocationFindRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/PeerLocationFindResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/SessionCreateRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/SessionCreateResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/SessionDeleteRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/SessionDeleteResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/SessionKeepAliveRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-finder/cpp/SessionKeepAliveResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-salt/cpp/MessageFactoryPeerSalt.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-salt/cpp/SignedSaltGetRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-salt/cpp/SignedSaltGetResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-to-peer/cpp/MessageFactoryPeerToPeer.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-to-peer/cpp/PeerIdentifyRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-to-peer/cpp/PeerIdentifyResult.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-to-peer/cpp/PeerKeepAliveRequest.cpp \
		   $(MESSAGE_SOURCE_PATH)/peer-to-peer/cpp/PeerKeepAliveResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/AccessRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/AccessResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/ChangedNotify.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/FolderGetRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/FolderGetResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/FolderUpdateRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/FolderUpdateResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/FoldersGetRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/FoldersGetResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/ListFetchRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/ListFetchResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/MessageFactoryPushMailbox.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/MessageUpdateRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/MessageUpdateResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/MessagesDataGetRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/MessagesDataGetResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/MessagesMetaDataGetRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/MessagesMetaDataGetResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/NamespaceGrantChallengeValidateRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/NamespaceGrantChallengeValidateResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/PeerValidateRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/PeerValidateResult.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/RegisterPushRequest.cpp \
$(MESSAGE_SOURCE_PATH)/push-mailbox/cpp/RegisterPushResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/rolodex/cpp/MessageFactoryRolodex.cpp \
		  $(MESSAGE_SOURCE_PATH)/rolodex/cpp/RolodexAccessRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/rolodex/cpp/RolodexAccessResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/rolodex/cpp/RolodexContactsGetRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/rolodex/cpp/RolodexContactsGetResult.cpp \
		  $(MESSAGE_SOURCE_PATH)/rolodex/cpp/RolodexNamespaceGrantChallengeValidateRequest.cpp \
		  $(MESSAGE_SOURCE_PATH)/rolodex/cpp/RolodexNamespaceGrantChallengeValidateResult.cpp


include $(BUILD_STATIC_LIBRARY)

