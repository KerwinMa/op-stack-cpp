/*

 Copyright (c) 2014, Hookflash Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#include <openpeer/stack/internal/stack_LocationDatabaseAbstraction.h>
#include <openpeer/stack/internal/stack_ILocationDatabaseAbstractionTables.h>

#include <openpeer/stack/internal/stack_Helper.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IBackOffTimer.h>

#include <zsLib/helpers.h>
#include <zsLib/XML.h>

#include <easySQLite/SqlFieldSet.h>

#define OPENPEER_STACK_LOCATION_DATABASE_VERSION 1

#define OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_PEER_LOCATION_DOWNLOADED_BUT_NOT_NOTIFIED_BATCH_LIMIT 10
#define OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_PEER_LOCATION_UNUSED_BATCH_LIMIT 10

#define OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_BY_PEER_LOCATION_INDEX_BATCH_LIMIT 10
#define OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_WITH_PEER_URI_PERMISSION_BATCH_LIMIT 10
#define OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_DOWNLOADED_BUT_NOT_NOTIFIED_BATCH_LIMIT 10

#define OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_CHANGE_WITH_PEER_URI_PERMISSION_BATCH_LIMIT 10

#define OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_ENTRY_BATCH_LIMIT 10

#define OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_ENTRY_CHANGE_BATCH_LIMIT 10

//#define OPENPEER_STACK_PUSH_MAILBOX_DATABASE_VERSION 1
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_FOLDERS_NEEDING_UPDATE_LIMIT 5
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_FOLDER_UNIQUE_ID_LENGTH 16
//
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_FOLDER_VERSIONED_MESSAGES_BATCH_LIMIT 10
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_MESSAGES_NEEDING_UPDATE_BATCH_LIMIT 10
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_MESSAGES_NEEDING_DATA_BATCH_LIMIT 10
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_MESSAGES_NEEDING_KEY_PROCESSING_BATCH_LIMIT 10
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_MESSAGES_NEEDING_DECRYPTING_BATCH_LIMIT 10
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_MESSAGES_NEEDING_NOTIFICATION_BATCH_LIMIT 10
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_PENDING_DELIVERY_MESSAGES_BATCH_LIMIT 10
//#define OPENPEER_STACK_SERVICES_PUSH_MAILBOX_DATABASE_ABSTRACTION_LIST_NEEDING_DOWNLOAD_BATCH_LIMIT 10

namespace openpeer { namespace stack { ZS_DECLARE_SUBSYSTEM(openpeer_stack) } }

namespace openpeer
{
  namespace stack
  {
    namespace internal
    {
      ZS_DECLARE_TYPEDEF_PTR(services::IHelper, UseServicesHelper)
      ZS_DECLARE_TYPEDEF_PTR(Helper, UseStackHelper)

      ZS_DECLARE_TYPEDEF_PTR(services::ISettings, UseSettings)
      ZS_DECLARE_TYPEDEF_PTR(services::IBackOffTimer, UseBackOffTimer)

      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstractionTables, UseTables)

      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::PeerLocationRecord, PeerLocationRecord)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::DatabaseRecord, DatabaseRecord)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::DatabaseChangeRecord, DatabaseChangeRecord)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::DatabaseChangeRecordList, DatabaseChangeRecordList)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::EntryRecord, EntryRecord)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::EntryChangeRecord, EntryChangeRecord)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::EntryChangeRecordList, EntryChangeRecordList)

      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::PeerLocationRecordList, PeerLocationRecordList)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::DatabaseRecordList, DatabaseRecordList)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::EntryRecordList, EntryRecordList)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::EntryChangeRecord, EntryChangeRecord)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::PeerURIList, PeerURIList)

      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::IPeerLocationTable, IPeerLocationTable)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::IDatabaseTable, IDatabaseTable)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::IDatabaseChangeTable, IDatabaseChangeTable)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::IPermissionTable, IPermissionTable)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::IEntryTable, IEntryTable)
      ZS_DECLARE_TYPEDEF_PTR(ILocationDatabaseAbstraction::IEntryChangeTable, IEntryChangeTable)

      ZS_DECLARE_TYPEDEF_PTR(sql::Table, SqlTable)
      ZS_DECLARE_TYPEDEF_PTR(sql::Record, SqlRecord)
      ZS_DECLARE_TYPEDEF_PTR(sql::RecordSet, SqlRecordSet)
      ZS_DECLARE_TYPEDEF_PTR(sql::Field, SqlField)
      ZS_DECLARE_TYPEDEF_PTR(sql::FieldSet, SqlFieldSet)
      ZS_DECLARE_TYPEDEF_PTR(sql::Value, SqlValue)

      typedef LocationDatabaseAbstraction::index index;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (helpers)
      #pragma mark

      //-----------------------------------------------------------------------
      static void ignoreAllFieldsInTable(
                                         SqlTable &table,
                                         bool ignored = true
                                         )
      {
        SqlFieldSet *fields = table.fields();
        
        for (int loop = 0; loop < fields->count(); ++loop) {
          SqlField *field = fields->getByIndex(loop);
          field->setIgnored(ignored);
        }
      }
      
      //-----------------------------------------------------------------------
      static void initializeFields(SqlRecord &record)
      {
        const SqlFieldSet *fields = record.fields();
        for (int loop = 0; loop < fields->count(); ++loop)
        {
          const SqlField *field = fields->getByIndex(loop);
          if (field->isIgnored()) continue;
          if (!field->isNotNull()) continue;

          SqlValue *value = record.getValue(loop);
          if (value->isIgnored()) continue;
          if (value->hasData()) continue;
          
          switch (field->getType()) {
            case sql::type_undefined: break;
            case sql::type_int:       value->setInteger(0);
            case sql::type_text:      value->setString(String());
            case sql::type_float:     value->setDouble(0.0);
            case sql::type_bool:      value->setBool(false);
            case sql::type_time:      value->setTime(0);
          }
        }
      }
      
      //-----------------------------------------------------------------------
      static void convert(
                          SqlRecord *record,
                          PeerLocationRecord &output
                          )
      {
        output.mIndex = static_cast<decltype(output.mIndex)>(record->getValue(SqlField::id)->asInteger());
        output.mPeerURI = record->getValue(UseTables::peerURI)->asString();
        output.mLocationID = record->getValue(UseTables::locationID)->asString();
        output.mLastDownloadedVersion = record->getValue(UseTables::lastDownloadedVersion)->asString();
        output.mDownloadComplete = record->getValue(UseTables::downloadComplete)->asBool();
        output.mNotified = record->getValue(UseTables::notified)->asBool();
        auto lastAccessed = record->getValue(UseTables::lastAccessed)->asInteger();
        if (0 != lastAccessed) {
          output.mLastAccessed = zsLib::timeSinceEpoch(Seconds(lastAccessed));
        } else {
          output.mLastAccessed = Time();
        }
      }

      //-----------------------------------------------------------------------
      static void convert(
                          SqlRecord *record,
                          DatabaseRecord &output
                          )
      {
        output.mIndex = static_cast<decltype(output.mIndex)>(record->getValue(SqlField::id)->asInteger());
        output.mIndexPeerLocation = static_cast<decltype(output.mIndexPeerLocation)>(record->getValue(UseTables::indexPeerLocation)->asInteger());
        output.mDatabaseID = record->getValue(UseTables::databaseID)->asString();
        output.mLastDownloadedVersion = record->getValue(UseTables::lastDownloadedVersion)->asString();
        String metaData = record->getValue(UseTables::metaData)->asString();
        if (metaData.hasData()) {
          output.mMetaData = UseServicesHelper::toJSON(metaData);
        }
        auto expires = record->getValue(UseTables::expires)->asInteger();
        if (0 != expires) {
          output.mExpires = zsLib::timeSinceEpoch(Seconds(expires));
        } else {
          output.mExpires = Time();
        }
        output.mDownloadComplete = record->getValue(UseTables::downloadComplete)->asBool();
        output.mNotified = record->getValue(UseTables::notified)->asBool();
      }

      //-----------------------------------------------------------------------
      static void convert(
                          SqlRecord *record,
                          DatabaseChangeRecord &output
                          )
      {
        output.mIndex = static_cast<decltype(output.mIndex)>(record->getValue(SqlField::id)->asInteger());
        output.mIndexPeerLocation = static_cast<decltype(output.mIndexPeerLocation)>(record->getValue(UseTables::indexPeerLocation)->asInteger());
        output.mDisposition = static_cast<decltype(output.mDisposition)>(record->getValue(UseTables::disposition)->asInteger());
        output.mIndexDatabase = static_cast<decltype(output.mIndexPeerLocation)>(record->getValue(UseTables::indexDatabase)->asInteger());
      }

      //-----------------------------------------------------------------------
      static void convert(
                          SqlRecord *record,
                          EntryRecord &output
                          )
      {
        output.mIndex = static_cast<decltype(output.mIndex)>(record->getValue(SqlField::id)->asInteger());
        output.mEntryID = record->getValue(UseTables::entryID)->asString();
        output.mVersion = static_cast<decltype(output.mVersion)>(record->getValue(UseTables::version)->asInteger());
        String metaData = record->getValue(UseTables::metaData)->asString();
        if (metaData.hasData()) {
          output.mMetaData = UseServicesHelper::toJSON(metaData);
        }
        String data = record->getValue(UseTables::data)->asString();
        if (data.hasData()) {
          output.mData = UseServicesHelper::toJSON(data);
        }
        output.mDataLength = static_cast<decltype(output.mDataLength)>(record->getValue(UseTables::dataLength)->asInteger());
        if (0 == output.mDataLength) {
          output.mDataLength = data.length();
        }
        output.mDataFetched = static_cast<decltype(output.mDataFetched)>(record->getValue(UseTables::dataFetched)->asBool());
        if (data.hasData()) {
          output.mDataFetched = true;
        }
        auto created = record->getValue(UseTables::created)->asInteger();
        if (0 != created) {
          output.mCreated = zsLib::timeSinceEpoch(Seconds(created));
        } else {
          output.mCreated = Time();
        }
        auto updated = record->getValue(UseTables::updated)->asInteger();
        if (0 != updated) {
          output.mUpdated = zsLib::timeSinceEpoch(Seconds(updated));
        } else {
          output.mUpdated = Time();
        }
      }

      //-----------------------------------------------------------------------
      static void convert(
                          SqlRecord *record,
                          EntryChangeRecord &output
                          )
      {
        output.mIndex = static_cast<decltype(output.mIndex)>(record->getValue(SqlField::id)->asInteger());
        output.mDisposition = static_cast<decltype(output.mDisposition)>(record->getValue(UseTables::disposition)->asInteger());
        output.mEntryID = record->getValue(UseTables::entryID)->asString();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ILocationDatabaseAbstractionTypes::PeerLocationRecord
      #pragma mark
      
      //-----------------------------------------------------------------------
      ElementPtr ILocationDatabaseAbstractionTypes::PeerLocationRecord::toDebug() const
      {
        ElementPtr resultEl = Element::create("PeerLocationRecord");

        UseServicesHelper::debugAppend(resultEl, "index", mIndex);
        UseServicesHelper::debugAppend(resultEl, "peer uri", mPeerURI);
        UseServicesHelper::debugAppend(resultEl, "location id", mLocationID);
        UseServicesHelper::debugAppend(resultEl, "last downloaded version", mLastDownloadedVersion);
        UseServicesHelper::debugAppend(resultEl, "download complete", mDownloadComplete);
        UseServicesHelper::debugAppend(resultEl, "notified", mNotified);
        UseServicesHelper::debugAppend(resultEl, "last accessed", mLastAccessed);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ILocationDatabaseAbstractionTypes::DatabaseRecord
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr ILocationDatabaseAbstractionTypes::DatabaseRecord::toDebug() const
      {
        ElementPtr resultEl = Element::create("DatabaseRecord");
        
        UseServicesHelper::debugAppend(resultEl, "index", mIndex);
        UseServicesHelper::debugAppend(resultEl, "index peer location", mIndexPeerLocation);
        UseServicesHelper::debugAppend(resultEl, "database id", mDatabaseID);
        UseServicesHelper::debugAppend(resultEl, "last downloaded version", mLastDownloadedVersion);
        UseServicesHelper::debugAppend(resultEl, "metadata", mMetaData ? UseServicesHelper::toString(mMetaData) : String());
        UseServicesHelper::debugAppend(resultEl, "expires", mExpires);
        UseServicesHelper::debugAppend(resultEl, "download complete", mDownloadComplete);
        UseServicesHelper::debugAppend(resultEl, "notified", mNotified);

        return resultEl;
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ILocationDatabaseAbstractionTypes::DatabaseChangeRecord
      #pragma mark

      //-----------------------------------------------------------------------
      const char *ILocationDatabaseAbstractionTypes::DatabaseChangeRecord::toString(Dispositions disposition)
      {
        switch (disposition) {
          case Disposition_None:    return "none";
          case Disposition_Add:     return "add";
          case Disposition_Update:  return "update";
          case Disposition_Remove:  return "remove";
        }
        return "unknown";
      }

      //-----------------------------------------------------------------------
      ElementPtr ILocationDatabaseAbstractionTypes::DatabaseChangeRecord::toDebug() const
      {
        ElementPtr resultEl = Element::create("DatabaseChangeRecord");
        
        UseServicesHelper::debugAppend(resultEl, "index", mIndex);
        UseServicesHelper::debugAppend(resultEl, "index peer location", mIndexPeerLocation);
        UseServicesHelper::debugAppend(resultEl, "disposition", toString(mDisposition));
        UseServicesHelper::debugAppend(resultEl, "index database", mIndexDatabase);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ILocationDatabaseAbstractionTypes::EntryRecord
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr ILocationDatabaseAbstractionTypes::EntryRecord::toDebug() const
      {
        ElementPtr resultEl = Element::create("EntryRecord");
        
        UseServicesHelper::debugAppend(resultEl, "index", mIndex);
        UseServicesHelper::debugAppend(resultEl, "entry id", mEntryID);
        UseServicesHelper::debugAppend(resultEl, "version", mVersion);
        UseServicesHelper::debugAppend(resultEl, "metadata", mMetaData ? UseServicesHelper::toString(mMetaData) : String());
        UseServicesHelper::debugAppend(resultEl, "data", (bool)mData);
        UseServicesHelper::debugAppend(resultEl, "data length", mDataLength);
        UseServicesHelper::debugAppend(resultEl, "data fetch", mDataFetched);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ILocationDatabaseAbstractionTypes::EntryChangeRecord
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr ILocationDatabaseAbstractionTypes::EntryChangeRecord::toDebug() const
      {
        ElementPtr resultEl = Element::create("EntryChangeRecord");

        UseServicesHelper::debugAppend(resultEl, "index", mIndex);
        UseServicesHelper::debugAppend(resultEl, "disposition", toString(mDisposition));
        UseServicesHelper::debugAppend(resultEl, "entry id", mEntryID);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      const char *ILocationDatabaseAbstractionTypes::EntryChangeRecord::toString(Dispositions disposition)
      {
        switch (disposition) {
          case Disposition_None:    return "none";
          case Disposition_Add:     return "add";
          case Disposition_Update:  return "update";
          case Disposition_Remove:  return "remove";
        }
        return "unknown";
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction
      #pragma mark

      //-----------------------------------------------------------------------
      LocationDatabaseAbstraction::LocationDatabaseAbstraction(
                                                               const char *inHashRepresentingUser,
                                                               const char *inUserStorageFilePath
                                                               ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        mUserHash(inHashRepresentingUser),
        mStoragePath(inUserStorageFilePath)
      {
        ZS_THROW_INVALID_ARGUMENT_IF((mUserHash.isEmpty()) && (mStoragePath.isEmpty()))

        ZS_LOG_BASIC(log("created"))
      }

      //-----------------------------------------------------------------------
      LocationDatabaseAbstraction::LocationDatabaseAbstraction(
                                                               ILocationDatabaseAbstractionPtr inMasterDatabase,
                                                               const char *peerURI,
                                                               const char *locationID,
                                                               const char *databaseID
                                                               ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        mPeerURI(peerURI),
        mLocationID(locationID),
        mDatabaseID(databaseID)
      {
        ZS_THROW_INVALID_ARGUMENT_IF(!inMasterDatabase)

        LocationDatabaseAbstractionPtr masterDatabase = LocationDatabaseAbstraction::convert(inMasterDatabase);
        ZS_THROW_INVALID_ASSUMPTION_IF(!masterDatabase)

        mMaster = masterDatabase;
        mUserHash = masterDatabase->getUserHash();
        mStoragePath = masterDatabase->getUserStoragePath();

        ZS_THROW_INVALID_ARGUMENT_IF((mUserHash.isEmpty()) && (mStoragePath.isEmpty()))

        ZS_LOG_BASIC(log("created"))
      }
      
      //-----------------------------------------------------------------------
      LocationDatabaseAbstraction::~LocationDatabaseAbstraction()
      {
        if(isNoop()) return;
        
        mThisWeak.reset();
        ZS_LOG_BASIC(log("destroyed"))
        cancel();
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::init()
      {
        AutoRecursiveLock lock(*this);
        prepareDB();
      }

      //-----------------------------------------------------------------------
      LocationDatabaseAbstractionPtr LocationDatabaseAbstraction::convert(ILocationDatabaseAbstractionPtr session)
      {
        return ZS_DYNAMIC_PTR_CAST(LocationDatabaseAbstraction, session);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => ILocationDatabaseAbstraction
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr LocationDatabaseAbstraction::toDebug(ILocationDatabaseAbstractionPtr session)
      {
        if (!session) return ElementPtr();
        return LocationDatabaseAbstraction::convert(session)->toDebug();
      }

      //-----------------------------------------------------------------------
      LocationDatabaseAbstractionPtr LocationDatabaseAbstraction::createMaster(
                                                                               const char *inHashRepresentingUser,
                                                                               const char *inUserStorageFilePath
                                                                               )
      {
        LocationDatabaseAbstractionPtr pThis(new LocationDatabaseAbstraction(inHashRepresentingUser, inUserStorageFilePath));
        pThis->mThisWeak = pThis;
        pThis->init();
        if (!pThis->mDB) {
          ZS_LOG_ERROR(Basic, pThis->log("unable to create database"))
          return LocationDatabaseAbstractionPtr();
        }
        return pThis;
      }

      //-----------------------------------------------------------------------
      LocationDatabaseAbstractionPtr LocationDatabaseAbstraction::openDatabase(
                                                                               ILocationDatabaseAbstractionPtr masterDatabase,
                                                                               const char *peerURI,
                                                                               const char *locationID,
                                                                               const char *databaseID
                                                                               )
      {
        LocationDatabaseAbstractionPtr pThis(new LocationDatabaseAbstraction(masterDatabase, peerURI, locationID, databaseID));
        pThis->mThisWeak = pThis;
        pThis->init();
        if (!pThis->mDB) {
          ZS_LOG_ERROR(Basic, pThis->log("unable to create database"))
          return LocationDatabaseAbstractionPtr();
        }
        return pThis;
      }

      //-----------------------------------------------------------------------
      PUID LocationDatabaseAbstraction::getID() const
      {
        return mID;
      }

      //-----------------------------------------------------------------------
      IPeerLocationTablePtr LocationDatabaseAbstraction::peerLocationTable() const
      {
        return PeerLocationTable::create(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      IDatabaseTablePtr LocationDatabaseAbstraction::databaseTable() const
      {
        return DatabaseTable::create(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      IDatabaseChangeTablePtr LocationDatabaseAbstraction::databaseChangeTable() const
      {
        return DatabaseChangeTable::create(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      IPermissionTablePtr LocationDatabaseAbstraction::permissionTable() const
      {
        return PermissionTable::create(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      IEntryTablePtr LocationDatabaseAbstraction::entryTable() const
      {
        return EntryTable::create(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      IEntryChangeTablePtr LocationDatabaseAbstraction::entryChangeTable() const
      {
        return EntryChangeTable::create(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => (friend location)
      #pragma mark

      //-----------------------------------------------------------------------
      String LocationDatabaseAbstraction::getUserHash() const
      {
        return mUserHash;
      }

      //-----------------------------------------------------------------------
      String LocationDatabaseAbstraction::getUserStoragePath() const
      {
        return mStoragePath;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => IPeerLocationTable
      #pragma mark

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IPeerLocationTable_createOrObtain(
                                                                       const char *peerURI,
                                                                       const char *locationID,
                                                                       PeerLocationRecord &outRecord
                                                                       )
      {
        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::PeerLocation_name(), UseTables::PeerLocation());

          String peerLocationHash = UseServicesHelper::convertToHex(*UseServicesHelper::hash(String(peerURI) + ":" + String(locationID)));

          table.open(String(UseTables::peerLocationHash) + " = " + SqlQuote(peerLocationHash));

          SqlRecord *found = table.getTopRecord();

          if (found) {
            internal::convert(found, outRecord);
            ZS_LOG_TRACE(log("found peer location record") + outRecord.toDebug())
            return;
          }

          SqlRecord addRecord(table.fields());

          addRecord.setString(UseTables::peerLocationHash, peerLocationHash);
          addRecord.setString(UseTables::peerURI, peerURI);
          addRecord.setString(UseTables::locationID, locationID);
          addRecord.setString(UseTables::lastDownloadedVersion, String());
          addRecord.setBool(UseTables::downloadComplete, false);
          addRecord.setBool(UseTables::notified, false);

          Time now = zsLib::now();
          addRecord.setInteger(UseTables::lastAccessed, zsLib::timeSinceEpoch<Seconds>(now).count());

          ZS_LOG_TRACE(log("adding new peer location record") + UseStackHelper::toDebug(&table, &addRecord))
          table.addRecord(&addRecord);

          {
            table.open(String(UseTables::peerLocationHash) + " = " + SqlQuote(peerLocationHash));

            SqlRecord *found = table.getTopRecord();

            if (found) {
              internal::convert(found, outRecord);
              ZS_LOG_TRACE(log("found newly inserted peer location record") + outRecord.toDebug())
              return;
            }
          }

          ZS_THROW_BAD_STATE("did not found record just added")
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IPeerLocationTable_notifyDownloaded(
                                                                         index indexPeerLocationRecord,
                                                                         const char *downloadedToVersion,
                                                                         bool downloadComplete
                                                                         )
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("updating peer location downloaded") + ZS_PARAMIZE(indexPeerLocationRecord) + ZS_PARAMIZE(downloadedToVersion) + ZS_PARAMIZE(downloadComplete))

        try {

          SqlTable table(*mDB, UseTables::PeerLocation_name(), UseTables::PeerLocation());

          ignoreAllFieldsInTable(table);

          table.fields()->getByName(SqlField::id)->setIgnored(false);
          table.fields()->getByName(UseTables::lastDownloadedVersion)->setIgnored(false);
          table.fields()->getByName(UseTables::downloadComplete)->setIgnored(false);

          SqlRecord record(table.fields());

          record.setInteger(SqlField::id, indexPeerLocationRecord);
          record.setString(UseTables::lastDownloadedVersion, downloadedToVersion);
          record.setBool(UseTables::downloadComplete, downloadComplete);

          table.updateRecord(&record);

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      PeerLocationRecordListPtr LocationDatabaseAbstraction::IPeerLocationTable_getDownloadedButNotNotifiedBatch() const
      {
        AutoRecursiveLock lock(*this);

        PeerLocationRecordListPtr result(new PeerLocationRecordList);

        try {

          SqlTable table(*mDB, UseTables::PeerLocation_name(), UseTables::PeerLocation());

          table.open(String(UseTables::downloadComplete) + " = 1 AND " + UseTables::notified + " = 0 LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_PEER_LOCATION_DOWNLOADED_BUT_NOT_NOTIFIED_BATCH_LIMIT));

          if (table.recordCount() < 1) {
            ZS_LOG_TRACE(log("no peer location needing notification"))
            return result;
          }

          for (int loop = 0; loop < table.recordCount(); ++loop)
          {
            SqlRecord *record = table.getRecord(loop);
            if (!record) {
              ZS_LOG_WARNING(Detail, log("record is null from batch of messages needing update"))
              continue;
            }

            PeerLocationRecord peerLocationRecord;

            internal::convert(record, peerLocationRecord);

            ZS_LOG_TRACE(log("found downloaded peer location but not notified") + peerLocationRecord.toDebug())

            result->push_back(peerLocationRecord);
          }

          return  result;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        
        return PeerLocationRecordListPtr();
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IPeerLocationTable_notifyNotified(index indexPeerLocationRecord) const
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("updating peer location notified") + ZS_PARAMIZE(indexPeerLocationRecord))

        try {

          SqlTable table(*mDB, UseTables::PeerLocation_name(), UseTables::PeerLocation());

          ignoreAllFieldsInTable(table);

          table.fields()->getByName(SqlField::id)->setIgnored(false);
          table.fields()->getByName(UseTables::notified)->setIgnored(false);

          SqlRecord record(table.fields());

          record.setInteger(SqlField::id, indexPeerLocationRecord);
          record.setBool(UseTables::notified, true);

          table.updateRecord(&record);

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      PeerLocationRecordListPtr LocationDatabaseAbstraction::IPeerLocationTable_getUnusedLocationsBatch(const Time &lastAccessedBefore) const
      {
        AutoRecursiveLock lock(*this);

        PeerLocationRecordListPtr result(new PeerLocationRecordList);

        if (Time() == lastAccessedBefore) {
          ZS_LOG_WARNING(Detail, log("last accessed before is not legal") + ZS_PARAMIZE(lastAccessedBefore))
          return result;
        }

        try {

          SqlTable table(*mDB, UseTables::PeerLocation_name(), UseTables::PeerLocation());

          Seconds accessedBefore = zsLib::timeSinceEpoch<Seconds>(lastAccessedBefore);

          table.open(String(UseTables::lastAccessed) + " < " + string(accessedBefore.count()) + " LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_PEER_LOCATION_UNUSED_BATCH_LIMIT));

          if (table.recordCount() < 1) {
            ZS_LOG_TRACE(log("no unused locations found"))
            return result;
          }

          for (int loop = 0; loop < table.recordCount(); ++loop)
          {
            SqlRecord *record = table.getRecord(loop);
            if (!record) {
              ZS_LOG_WARNING(Detail, log("record is null from peer location last access batch"))
              continue;
            }

            PeerLocationRecord peerLocationRecord;

            internal::convert(record, peerLocationRecord);

            ZS_LOG_TRACE(log("found unused") + peerLocationRecord.toDebug())

            result->push_back(peerLocationRecord);
          }

          return  result;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }

        return PeerLocationRecordListPtr();
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IPeerLocationTable_remove(index indexPeerLocationRecord)
      {
        AutoRecursiveLock lock(*this);

        try {
          ZS_LOG_TRACE(log("deleting peer location record") + ZS_PARAMIZE(indexPeerLocationRecord))

          SqlTable table(*mDB, UseTables::PeerLocation_name(), UseTables::PeerLocation());

          table.deleteRecords(String(SqlField::id) + " = " + string(indexPeerLocationRecord));

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => IDatabaseTable
      #pragma mark

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IDatabaseTable_flushAllForPeerLocation(index indexPeerLocationRecord)
      {
        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          ZS_LOG_TRACE(log("removing all database records for peer location") + ZS_PARAMIZE(indexPeerLocationRecord))

          table.deleteRecords(String(UseTables::indexPeerLocation) + " = " + string(indexPeerLocationRecord));
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IDatabaseTable_addOrUpdate(
                                                                   DatabaseRecord &ioRecord,
                                                                   DatabaseChangeRecord &outChangeRecord
                                                                   )
      {
        AutoRecursiveLock lock(*this);

        outChangeRecord.mDisposition = DatabaseChangeRecord::Disposition_None;

        try {
          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          outChangeRecord.mIndexPeerLocation = ioRecord.mIndexPeerLocation;

          table.open(String(UseTables::indexPeerLocation) + " = " + string(ioRecord.mIndexPeerLocation) + " AND " + UseTables::databaseID + " = " + SqlQuote(ioRecord.mDatabaseID));

          SqlRecord *found = table.getTopRecord();

          if (found) {
            DatabaseRecord previousRecord;
            internal::convert(found, previousRecord);

            ZS_LOG_TRACE(log("found database record") + previousRecord.toDebug())

            bool changed = false;

            ioRecord.mIndex = previousRecord.mIndex;
            ioRecord.mIndexPeerLocation = previousRecord.mIndexPeerLocation;

            ignoreAllFieldsInTable(table);
            table.fields()->getByName(SqlField::id)->setIgnored(false);

            if (ioRecord.mLastDownloadedVersion != previousRecord.mLastDownloadedVersion) {
              table.fields()->getByName(UseTables::lastDownloadedVersion)->setIgnored(false);
              found->setString(UseTables::lastDownloadedVersion, ioRecord.mLastDownloadedVersion);
              changed = true;
            }

            if (ioRecord.mMetaData) {
              String metaDataAsStr = UseServicesHelper::toString(ioRecord.mMetaData);
              String previousAsStr = UseServicesHelper::toString(previousRecord.mMetaData);
              if (metaDataAsStr != previousAsStr) {
                table.fields()->getByName(UseTables::metaData)->setIgnored(false);
                found->setString(UseTables::metaData, metaDataAsStr);
                changed = true;
              }
            }

            auto countNew = zsLib::timeSinceEpoch<Seconds>(ioRecord.mExpires).count();
            auto countOld = zsLib::timeSinceEpoch<Seconds>(previousRecord.mExpires).count();

            if (Time() == ioRecord.mExpires) {
              countNew = 0;
            }
            if (Time() == previousRecord.mExpires) {
              countOld = 0;
            }

            if (countNew != countOld) {
              table.fields()->getByName(UseTables::expires)->setIgnored(false);
              changed = true;

              if (Time() != ioRecord.mExpires) {
                found->setInteger(UseTables::expires, zsLib::timeSinceEpoch<Seconds>(ioRecord.mExpires).count());
              } else {
                found->setInteger(UseTables::expires, 0);
              }
            }

            if (ioRecord.mDownloadComplete != previousRecord.mDownloadComplete) {
              table.fields()->getByName(UseTables::downloadComplete)->setIgnored(false);
              found->setBool(UseTables::downloadComplete, ioRecord.mDownloadComplete);
            }

            if (changed) {
              outChangeRecord.mDisposition = DatabaseChangeRecord::Disposition_Update;

              table.fields()->getByName(UseTables::notified)->setIgnored(false);
              found->setBool(UseTables::notified, false);
              ioRecord.mNotified = false;

              ZS_LOG_TRACE(log("updating existing record") + previousRecord.toDebug() + ioRecord.toDebug())
              table.updateRecord(found);
            }
            outChangeRecord.mIndexDatabase = previousRecord.mIndex;
            return;
          }

          SqlRecord addRecord(table.fields());

          addRecord.setInteger(UseTables::indexPeerLocation, ioRecord.mIndexPeerLocation);
          addRecord.setString(UseTables::databaseID, ioRecord.mDatabaseID);
          addRecord.setString(UseTables::lastDownloadedVersion, ioRecord.mLastDownloadedVersion);
          addRecord.setString(UseTables::metaData, UseServicesHelper::toString(ioRecord.mMetaData));
          if (Time() != ioRecord.mExpires) {
            addRecord.setInteger(UseTables::expires, zsLib::timeSinceEpoch<Seconds>(ioRecord.mExpires).count());
          } else {
            addRecord.setInteger(UseTables::expires, 0);
          }
          addRecord.setBool(UseTables::downloadComplete, ioRecord.mDownloadComplete);
          addRecord.setBool(UseTables::notified, false);

          ioRecord.mNotified = false;

          ZS_LOG_TRACE(log("adding new peer location record") + UseStackHelper::toDebug(&table, &addRecord))
          table.addRecord(&addRecord);

          {
            table.open(String(UseTables::indexPeerLocation) + " = " + string(ioRecord.mIndexPeerLocation) + " AND " + UseTables::databaseID + " = " + SqlQuote(ioRecord.mDatabaseID));

            SqlRecord *found = table.getTopRecord();

            if (found) {
              internal::convert(found, ioRecord);

              outChangeRecord.mDisposition = DatabaseChangeRecord::Disposition_Add;
              outChangeRecord.mIndexDatabase = ioRecord.mIndex;

              ZS_LOG_TRACE(log("found newly inserted database record") + ioRecord.toDebug())
              return;
            }
          }

          ZS_THROW_BAD_STATE("did not found record just added")
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      bool LocationDatabaseAbstraction::IDatabaseTable_remove(
                                                              const DatabaseRecord &inRecord,
                                                              DatabaseChangeRecord &outChangeRecord
                                                              )
      {
        AutoRecursiveLock lock(*this);

        outChangeRecord.mDisposition = DatabaseChangeRecord::Disposition_None;

        try {
          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != inRecord.mIndex) {
            table.open(String(SqlField::id) + " = " + string(inRecord.mIndex));
          } else {
            table.open(String(UseTables::indexPeerLocation) + " = " + string(inRecord.mIndexPeerLocation) + " AND " + UseTables::databaseID + " = " + SqlQuote(inRecord.mDatabaseID));
          }

          SqlRecord *found = table.getTopRecord();

          if (!found) {
            ZS_LOG_WARNING(Trace, log("did not fine existing database record") + inRecord.toDebug())
            return false;
          }

          DatabaseRecord previousRecord;
          internal::convert(found, previousRecord);

          outChangeRecord.mDisposition = DatabaseChangeRecord::Disposition_Remove;
          outChangeRecord.mIndexDatabase = previousRecord.mIndex;
          outChangeRecord.mIndexPeerLocation = previousRecord.mIndexPeerLocation;

          ZS_LOG_TRACE(log("removing previous database record") + previousRecord.toDebug())

          table.deleteRecords(String(SqlField::id) + " = " + string(previousRecord.mIndex));
          return true;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        return false;
      }

      //-----------------------------------------------------------------------
      bool LocationDatabaseAbstraction::IDatabaseTable_getByIndex(
                                                                  index indexDatabaseRecord,
                                                                  DatabaseRecord &outRecord
                                                                  ) const
      {
        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          table.open(String(SqlField::id) + " = " + string(indexDatabaseRecord));

          SqlRecord *found = table.getTopRecord();

          if (!found) {
            ZS_LOG_WARNING(Trace, log("did not fine existing database record") + ZS_PARAMIZE(indexDatabaseRecord))
            return false;
          }

          internal::convert(found, outRecord);

          ZS_LOG_TRACE(log("found previous database record") + outRecord.toDebug())

          return true;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        return false;
      }

      //-----------------------------------------------------------------------
      DatabaseRecordListPtr LocationDatabaseAbstraction::IDatabaseTable_getBatchByPeerLocationIndex(
                                                                                                 index indexPeerLocationRecord,
                                                                                                 index afterIndexDatabase
                                                                                                 ) const
      {
        AutoRecursiveLock lock(*this);

        DatabaseRecordListPtr result(new DatabaseRecordList);

        try {
          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          String afterClause;
          String orderClause = " ORDER BY " + String(SqlField::id) + " ASC";
          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != afterIndexDatabase) {
            afterClause = " AND " + String(SqlField::id) + " > " + string(afterIndexDatabase);
          }

          table.open(String(UseTables::indexPeerLocation) + " = " + string(indexPeerLocationRecord) + afterClause + orderClause + " LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_BY_PEER_LOCATION_INDEX_BATCH_LIMIT));

          for (int row = 0; row < table.recordCount(); ++row) {
            SqlRecord *record = table.getRecord(row);

            DatabaseRecord databaseRecord;
            internal::convert(record, databaseRecord);

            ZS_LOG_TRACE(log("found database record") + databaseRecord.toDebug())

            result->push_back(databaseRecord);
          }

          return result;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }

        return DatabaseRecordListPtr();
      }

      //-----------------------------------------------------------------------
      DatabaseRecordListPtr LocationDatabaseAbstraction::IDatabaseTable_getBatchByPeerLocationIndexForPeerURI(
                                                                                                              const char *peerURIWithPermission,
                                                                                                              index indexPeerLocationRecord,
                                                                                                              index afterIndexDatabase
                                                                                                              ) const
      {
        AutoRecursiveLock lock(*this);

        DatabaseRecordListPtr result(new DatabaseRecordList);

        try {
          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          String afterClause;
          String orderClause = " ORDER BY " + table.name() + "." + SqlField::id + " ASC";
          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != afterIndexDatabase) {
            afterClause = " AND " + table.name() + "." + SqlField::id + " > " + string(afterIndexDatabase);
          }

          String whereID = table.name() + "." + UseTables::indexPeerLocation + " = " + string(indexPeerLocationRecord);
          String whereURI = " AND " + String(UseTables::Permission_name()) + "." + UseTables::peerURI + " = " + SqlQuote(String(peerURIWithPermission));

          String queryStr = "SELECT " + table.getSelectFields(true) + " FROM " + table.name() + " INNER JOIN " + UseTables::Permission_name() + " ON " + table.name() + "." + SqlField::id + " = " + UseTables::Permission_name() + "." + UseTables::indexDatabase + " WHERE ";
          String whereClause = whereID + whereURI + afterClause;

          queryStr += whereClause + orderClause + " LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_WITH_PEER_URI_PERMISSION_BATCH_LIMIT);

          SqlRecordSet recordSet(*mDB, table.fields());

          recordSet.query(queryStr);

          for (int row = 0; row < recordSet.count(); ++row) {
            SqlRecord *record = recordSet.getRecord(row);

            DatabaseRecord databaseRecord;
            internal::convert(record, databaseRecord);

            ZS_LOG_TRACE(log("found database record") + databaseRecord.toDebug())

            result->push_back(databaseRecord);
          }

          return result;

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        
        return DatabaseRecordListPtr();
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IDatabaseTable_notifyDownloaded(
                                                                        index indexDatabaseRecord,
                                                                        const char *downloadedToVersion,
                                                                        bool downloadComplete
                                                                        )
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("updating database downloaded") + ZS_PARAMIZE(indexDatabaseRecord) + ZS_PARAMIZE(downloadedToVersion) + ZS_PARAMIZE(downloadComplete))

        try {

          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          ignoreAllFieldsInTable(table);

          table.fields()->getByName(SqlField::id)->setIgnored(false);
          table.fields()->getByName(UseTables::lastDownloadedVersion)->setIgnored(false);
          table.fields()->getByName(UseTables::downloadComplete)->setIgnored(false);

          SqlRecord record(table.fields());

          record.setInteger(SqlField::id, indexDatabaseRecord);
          record.setString(UseTables::lastDownloadedVersion, downloadedToVersion);
          record.setBool(UseTables::downloadComplete, downloadComplete);

          table.updateRecord(&record);

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      DatabaseRecordListPtr LocationDatabaseAbstraction::IDatabaseTable_getDownloadedButNotNotifiedBatch() const
      {
        AutoRecursiveLock lock(*this);

        DatabaseRecordListPtr result(new DatabaseRecordList);

        try {

          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          table.open(String(UseTables::downloadComplete) + " = 1 AND " + UseTables::notified + " = 0 LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_DOWNLOADED_BUT_NOT_NOTIFIED_BATCH_LIMIT));

          if (table.recordCount() < 1) {
            ZS_LOG_TRACE(log("no database needing notification"))
            return result;
          }

          for (int loop = 0; loop < table.recordCount(); ++loop)
          {
            SqlRecord *record = table.getRecord(loop);
            if (!record) {
              ZS_LOG_WARNING(Detail, log("record is null from batch of messages needing update"))
              continue;
            }

            DatabaseRecord databaseRecord;

            internal::convert(record, databaseRecord);

            ZS_LOG_TRACE(log("found downloaded database but not notified") + databaseRecord.toDebug())

            result->push_back(databaseRecord);
          }

          return  result;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        return DatabaseRecordListPtr();
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IDatabaseTable_notifyNotified(index indexDatabaseRecord) const
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("updating database notified") + ZS_PARAMIZE(indexDatabaseRecord))

        try {

          SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());

          ignoreAllFieldsInTable(table);

          table.fields()->getByName(SqlField::id)->setIgnored(false);
          table.fields()->getByName(UseTables::notified)->setIgnored(false);

          SqlRecord record(table.fields());

          record.setInteger(SqlField::id, indexDatabaseRecord);
          record.setBool(UseTables::notified, true);

          table.updateRecord(&record);

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => IDatabaseChangeTable
      #pragma mark

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IDatabaseChangeTable_flushAllForPeerLocation(index indexPeerLocationRecord)
      {
        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::DatabaseChange_name(), UseTables::DatabaseChange());

          ZS_LOG_TRACE(log("removing all database change records for peer location") + ZS_PARAMIZE(indexPeerLocationRecord))

          table.deleteRecords(String(UseTables::indexPeerLocation) + " = " + string(indexPeerLocationRecord));
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IDatabaseChangeTable_flushAllForDatabase(index indexDatabase)
      {
        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::DatabaseChange_name(), UseTables::DatabaseChange());

          ZS_LOG_TRACE(log("removing all database change records for database") + ZS_PARAMIZE(indexDatabase))

          table.deleteRecords(String(UseTables::indexDatabase) + " = " + string(indexDatabase));
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IDatabaseChangeTable_insert(const DatabaseChangeRecord &record)
      {
        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::DatabaseChange_name(), UseTables::DatabaseChange());

          SqlRecord addRecord(table.fields());

          addRecord.setInteger(UseTables::indexPeerLocation, record.mIndexPeerLocation);
          addRecord.setInteger(UseTables::indexDatabase, record.mIndexDatabase);
          addRecord.setInteger(UseTables::disposition, (int) record.mDisposition);

          ZS_LOG_TRACE(log("adding new database change record") + UseStackHelper::toDebug(&table, &addRecord))
          table.addRecord(&addRecord);

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      DatabaseChangeRecordListPtr LocationDatabaseAbstraction::IDatabaseChangeTable_getChangeBatch(
                                                                                                   index indexPeerLocationRecord,
                                                                                                   index afterIndexDatabaseChange
                                                                                                   ) const
      {
        AutoRecursiveLock lock(*this);

        DatabaseChangeRecordListPtr result(new DatabaseChangeRecordList);

        try {
          SqlTable table(*mDB, UseTables::DatabaseChange_name(), UseTables::DatabaseChange());

          String afterClause;
          String orderClause = " ORDER BY " + String(SqlField::id) + " ASC";
          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != afterIndexDatabaseChange) {
            afterClause = " AND " + String(SqlField::id) + " > " + string(afterIndexDatabaseChange);
          }

          table.open(String(UseTables::indexPeerLocation) + " = " + string(indexPeerLocationRecord) + afterClause + orderClause + " LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_BY_PEER_LOCATION_INDEX_BATCH_LIMIT));

          for (int row = 0; row < table.recordCount(); ++row) {
            SqlRecord *record = table.getRecord(row);

            DatabaseChangeRecord databaseChangeRecord;
            internal::convert(record, databaseChangeRecord);

            ZS_LOG_TRACE(log("found database change record") + databaseChangeRecord.toDebug())

            result->push_back(databaseChangeRecord);
          }

          return result;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        
        return DatabaseChangeRecordListPtr();
      }

      //-----------------------------------------------------------------------
      DatabaseChangeRecordListPtr LocationDatabaseAbstraction::IDatabaseChangeTable_getChangeBatchForPeerURI(
                                                                                                             const char *peerURIWithPermission,
                                                                                                             index indexPeerLocationRecord,
                                                                                                             index afterIndexDatabaseChange
                                                                                                             ) const
      {
        AutoRecursiveLock lock(*this);

        DatabaseChangeRecordListPtr result(new DatabaseChangeRecordList);

        try {
          SqlTable table(*mDB, UseTables::DatabaseChange_name(), UseTables::DatabaseChange());

          String afterClause;
          String orderClause = " ORDER BY " + table.name() + "." + SqlField::id + " ASC";
          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != afterIndexDatabaseChange) {
            afterClause = " AND " + table.name() + "." + SqlField::id + " > " + string(afterIndexDatabaseChange);
          }

          String whereID = table.name() + "." + UseTables::indexPeerLocation + " = " + string(indexPeerLocationRecord);
          String whereURI = " AND " + String(UseTables::Permission_name()) + "." + UseTables::peerURI + " = " + SqlQuote(String(peerURIWithPermission));

          String queryStr = "SELECT " + table.getSelectFields(true) + " FROM " + table.name() + " INNER JOIN " + UseTables::Permission_name() + " ON (" + table.name() + "." + UseTables::indexPeerLocation + " = " + UseTables::Permission_name() + "." + UseTables::indexPeerLocation + " AND " + table.name() + "." + UseTables::indexDatabase + " = " + UseTables::Permission_name() + "." + UseTables::indexDatabase + ") WHERE ";
          String whereClause = whereID + whereURI + afterClause;

          queryStr += whereClause + orderClause + " LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_DATABASE_CHANGE_WITH_PEER_URI_PERMISSION_BATCH_LIMIT);

          SqlRecordSet recordSet(*mDB, table.fields());

          recordSet.query(queryStr);

          for (int row = 0; row < recordSet.count(); ++row) {
            SqlRecord *record = recordSet.getRecord(row);

            DatabaseChangeRecord databaseChangeRecord;
            internal::convert(record, databaseChangeRecord);

            ZS_LOG_TRACE(log("found database change record") + databaseChangeRecord.toDebug())

            result->push_back(databaseChangeRecord);
          }

          return result;

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        
        return DatabaseChangeRecordListPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => IPermissionTable
      #pragma mark

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IPermissionTable_flushAllForPeerLocation(index indexPeerLocationRecord)
      {
        AutoRecursiveLock lock(*this);

        try {
          ZS_LOG_TRACE(log("deleting database permission records") + ZS_PARAMIZE(indexPeerLocationRecord))

          SqlTable table(*mDB, UseTables::Permission_name(), UseTables::Permission());

          table.deleteRecords(String(UseTables::indexPeerLocation) + " = " + string(indexPeerLocationRecord));

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IPermissionTable_flushAllForDatabase(index indexDatabaseRecord)
      {
        AutoRecursiveLock lock(*this);

        try {
          ZS_LOG_TRACE(log("deleting database permission records") + ZS_PARAMIZE(indexDatabaseRecord))

          SqlTable table(*mDB, UseTables::Permission_name(), UseTables::Permission());

          table.deleteRecords(String(UseTables::indexDatabase) + " = " + string(indexDatabaseRecord));

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IPermissionTable_insert(
                                                                index indexPeerLocation,
                                                                index indexDatabaseRecord,
                                                                const PeerURIList &uris
                                                                )
      {
        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::Permission_name(), UseTables::Permission());

          for (auto iter = uris.begin(); iter != uris.end(); ++iter)
          {
            const PeerURI &uri = (*iter);

            if (uri.isEmpty()) {
              ZS_LOG_WARNING(Detail, log("blank URI detected in permission URI list"))
              continue;
            }

            SqlRecord addRecord(table.fields());

            addRecord.setInteger(UseTables::indexPeerLocation, indexPeerLocation);
            addRecord.setInteger(UseTables::indexDatabase, indexDatabaseRecord);
            addRecord.setString(UseTables::peerURI, uri);

            ZS_LOG_TRACE(log("adding new database permission record") + UseStackHelper::toDebug(&table, &addRecord))
            table.addRecord(&addRecord);
          }
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IPermissionTable_getByDatabaseIndex(
                                                                            index indexDatabaseRecord,
                                                                            PeerURIList &outURIs
                                                                            ) const
      {
        outURIs.clear();

        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::Permission_name(), UseTables::Permission());

          table.open(String(UseTables::indexDatabase) + " = " + string(indexDatabaseRecord));

          for (int row = 0; row < table.recordCount(); ++row) {
            SqlRecord *record = table.getRecord(row);

            String uri = record->getValue(UseTables::peerURI)->asString();

            ZS_LOG_TRACE(log("found database permission record") + ZS_PARAMIZE(uri))

            outURIs.push_back(uri);
          }
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => IEntryTable
      #pragma mark

      //-----------------------------------------------------------------------
      bool LocationDatabaseAbstraction::IEntryTable_add(
                                                        const EntryRecord &entry,
                                                        EntryChangeRecord &outChangeRecord
                                                        )
      {
        AutoRecursiveLock lock(*this);

        outChangeRecord.mDisposition = EntryChangeRecord::Disposition_None;
        outChangeRecord.mEntryID = entry.mEntryID;

        try {
          SqlTable table(*mDB, UseTables::Entry_name(), UseTables::Entry());

          SqlRecord addRecord(table.fields());

          addRecord.setString(UseTables::entryID, entry.mEntryID);
          addRecord.setInteger(UseTables::version, 0 == entry.mVersion ? 1 : entry.mVersion);
          addRecord.setString(UseTables::metaData, UseServicesHelper::toString(entry.mMetaData));

          String dataAsStr = UseServicesHelper::toString(entry.mData);
          addRecord.setString(UseTables::data, dataAsStr);
          addRecord.setInteger(UseTables::dataLength, dataAsStr.isEmpty() ? entry.mDataLength : dataAsStr.length());
          addRecord.setBool(UseTables::dataFetched, entry.mDataFetched);
          if (dataAsStr.hasData()) {
            addRecord.setBool(UseTables::dataFetched, true);
          } else if (0 == entry.mDataLength) {
            addRecord.setBool(UseTables::dataFetched, true);
          } else {
            addRecord.setBool(UseTables::dataFetched, false);
          }

          if (Time() != entry.mCreated) {
            addRecord.setInteger(UseTables::created, zsLib::timeSinceEpoch<Seconds>(entry.mCreated).count());
          } else {
            addRecord.setInteger(UseTables::created, 0);
          }
          if (Time() != entry.mUpdated) {
            addRecord.setInteger(UseTables::updated, zsLib::timeSinceEpoch<Seconds>(entry.mUpdated).count());
          } else {
            addRecord.setInteger(UseTables::updated, 0);
          }

          ZS_LOG_TRACE(log("adding new entry record") + UseStackHelper::toDebug(&table, &addRecord))

          table.addRecord(&addRecord);

          outChangeRecord.mDisposition = EntryChangeRecord::Disposition_Add;

          return true;

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        return false;
      }

      //-----------------------------------------------------------------------
      bool LocationDatabaseAbstraction::IEntryTable_update(
                                                           const EntryRecord &entry,
                                                           EntryChangeRecord &outChangeRecord
                                                           )
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("updating entry record") + entry.toDebug())

        outChangeRecord.mDisposition = EntryChangeRecord::Disposition_None;
        outChangeRecord.mEntryID = entry.mEntryID;

        try {

          SqlTable table(*mDB, UseTables::Entry_name(), UseTables::Entry());

          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != entry.mIndex) {
            table.open(String(SqlField::id) + " = " + string(entry.mIndex));
          } else {
            table.open(String(UseTables::entryID) + " = " + SqlQuote(entry.mEntryID));
          }

          SqlRecord *found = table.getTopRecord();

          if (!found) {
            ZS_LOG_TRACE(log("did not find an existing record to update") + entry.toDebug())
            return false;
          }

          EntryRecord previousEntry;
          internal::convert(found, previousEntry);

          bool changed = false;

          ignoreAllFieldsInTable(table);

          table.fields()->getByName(UseTables::version)->setIgnored(false);

          if (0 != entry.mVersion) {
            if (entry.mVersion <= previousEntry.mVersion) {
              ZS_LOG_WARNING(Detail, log("attempting to update with an older version of an entry record") + entry.toDebug() + previousEntry.toDebug())
              return false;
            }
            changed = true;

            found->setInteger(UseTables::version, entry.mVersion);
          } else {
            found->setInteger(UseTables::version, previousEntry.mVersion + 1);
          }

          String entryDataAsStr = UseServicesHelper::toString(entry.mData);
          String previousEntryDataAsStr = UseServicesHelper::toString(previousEntry.mData);

          if (entryDataAsStr != previousEntryDataAsStr) {
            changed = true;
            table.fields()->getByName(UseTables::data)->setIgnored(false);
            found->setString(UseTables::data, entryDataAsStr);
          }

          if (entryDataAsStr.hasData()) {
            if (entryDataAsStr.length() != previousEntry.mDataLength) {
              changed = true;
              table.fields()->getByName(UseTables::dataLength)->setIgnored(false);
              found->setInteger(UseTables::dataLength, entryDataAsStr.length());

              table.fields()->getByName(UseTables::dataFetched)->setIgnored(false);
              found->setBool(UseTables::dataFetched, true);
            }
          } else if (0 != entry.mDataLength) {
            if (entry.mDataLength != previousEntry.mDataLength) {
              changed = true;
              table.fields()->getByName(UseTables::data)->setIgnored(false);
              found->setString(UseTables::data, String());

              table.fields()->getByName(UseTables::dataLength)->setIgnored(false);
              found->setInteger(UseTables::dataLength, entry.mDataLength);

              table.fields()->getByName(UseTables::dataFetched)->setIgnored(false);
              found->setBool(UseTables::dataFetched, false);
            }
          } else {
            if (0 != previousEntry.mDataLength) {
              changed = true;
              table.fields()->getByName(UseTables::data)->setIgnored(false);
              found->setString(UseTables::data, String());

              table.fields()->getByName(UseTables::dataLength)->setIgnored(false);
              found->setInteger(UseTables::dataLength, entry.mDataLength);

              table.fields()->getByName(UseTables::dataFetched)->setIgnored(false);
              found->setBool(UseTables::dataFetched, true);
            }
          }

          if (Time() != entry.mCreated) {
            auto current = zsLib::timeSinceEpoch<Seconds>(entry.mCreated).count();
            auto previous = zsLib::timeSinceEpoch<Seconds>(previousEntry.mCreated).count();

            if (current != previous) {
              changed = true;
              table.fields()->getByName(UseTables::created)->setIgnored(false);
              found->setInteger(UseTables::created, current);
            }
          }

          if (Time() != entry.mUpdated) {
            auto current = zsLib::timeSinceEpoch<Seconds>(entry.mUpdated).count();
            auto previous = zsLib::timeSinceEpoch<Seconds>(previousEntry.mUpdated).count();

            if (current != previous) {
              changed = true;
              table.fields()->getByName(UseTables::updated)->setIgnored(false);
              found->setInteger(UseTables::updated, current);
            }
          }

          if (!changed) {
            ZS_LOG_TRACE(log("no changes found from previous record") + entry.toDebug() + previousEntry.toDebug())
            return false;
          }

          table.fields()->getByName(SqlField::id)->setIgnored(false);

          table.updateRecord(found);

          outChangeRecord.mDisposition = EntryChangeRecord::Disposition_Update;

          return true;

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        return false;
      }

      //-----------------------------------------------------------------------
      bool LocationDatabaseAbstraction::IEntryTable_remove(
                                                           const EntryRecord &entry,
                                                           EntryChangeRecord &outChangeRecord
                                                           )
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("updating entry record") + entry.toDebug())

        outChangeRecord.mDisposition = EntryChangeRecord::Disposition_None;
        outChangeRecord.mEntryID = entry.mEntryID;

        try {

          SqlTable table(*mDB, UseTables::Entry_name(), UseTables::Entry());

          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != entry.mIndex) {
            table.open(String(SqlField::id) + " = " + string(entry.mIndex));
          } else {
            table.open(String(UseTables::entryID) + " = " + SqlQuote(entry.mEntryID));
          }

          SqlRecord *found = table.getTopRecord();

          if (!found) {
            ZS_LOG_TRACE(log("did not find an existing record to remove") + entry.toDebug())
            return false;
          }

          table.deleteRecords(String(SqlField::id) + " = " + string(found->getValue(SqlField::id)->asInteger()));

          outChangeRecord.mDisposition = EntryChangeRecord::Disposition_Remove;

          return true;
          
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        return false;
      }
      
      //-----------------------------------------------------------------------
      EntryRecordListPtr LocationDatabaseAbstraction::IEntryTable_getBatch(
                                                                           bool includeData,
                                                                           index afterIndexEntry
                                                                           ) const
      {
        AutoRecursiveLock lock(*this);

        EntryRecordListPtr result(new EntryRecordList);

        try {
          SqlTable table(*mDB, UseTables::Entry_name(), UseTables::Entry());

          String afterClause;
          String orderClause = " ORDER BY " + String(SqlField::id) + " ASC";
          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != afterIndexEntry) {
            afterClause = String(SqlField::id) + " > " + string(afterIndexEntry);
          } else {
            afterClause = String(SqlField::id) + " > " + string(0);
          }

          if (!includeData) {
            table.fields()->getByName(UseTables::data)->setIgnored(true);
          }

          table.open(afterClause + orderClause + " LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_ENTRY_BATCH_LIMIT));

          for (int row = 0; row < table.recordCount(); ++row) {
            SqlRecord *record = table.getRecord(row);

            EntryRecord entryRecord;
            internal::convert(record, entryRecord);

            ZS_LOG_TRACE(log("found entry record") + entryRecord.toDebug())

            result->push_back(entryRecord);
          }

          return result;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        
        return EntryRecordListPtr();
      }

      //-----------------------------------------------------------------------
      bool LocationDatabaseAbstraction::IEntryTable_getEntry(
                                                             EntryRecord &ioEntry,
                                                             bool includeData
                                                             ) const
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("getting entry record") + ioEntry.toDebug())

        try {

          SqlTable table(*mDB, UseTables::Entry_name(), UseTables::Entry());

          if (!includeData) {
            table.fields()->getByName(UseTables::data)->setIgnored(true);
          }

          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != ioEntry.mIndex) {
            table.open(String(SqlField::id) + " = " + string(ioEntry.mIndex));
          } else {
            table.open(String(UseTables::entryID) + " = " + SqlQuote(ioEntry.mEntryID));
          }

          SqlRecord *found = table.getTopRecord();

          if (!found) {
            ZS_LOG_TRACE(log("did not find an existing entry record") + ioEntry.toDebug())
            return false;
          }

          internal::convert(found, ioEntry);

          return true;

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
        return false;
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => IEntryChangeTable
      #pragma mark

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::IEntryChangeTable_insert(const EntryChangeRecord &record)
      {
        AutoRecursiveLock lock(*this);

        try {
          SqlTable table(*mDB, UseTables::EntryChange_name(), UseTables::EntryChange());

          SqlRecord addRecord(table.fields());

          addRecord.setInteger(UseTables::disposition, record.mDisposition);
          addRecord.setString(UseTables::entryID, record.mEntryID);

          ZS_LOG_TRACE(log("adding new entry record") + UseStackHelper::toDebug(&table, &addRecord))

          table.addRecord(&addRecord);

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }
      }

      //-----------------------------------------------------------------------
      EntryChangeRecordListPtr LocationDatabaseAbstraction::IEntryChangeTable_getBatch(index afterIndexEntryChanged) const
      {
        AutoRecursiveLock lock(*this);

        EntryChangeRecordListPtr result(new EntryChangeRecordList);

        try {
          SqlTable table(*mDB, UseTables::EntryChange_name(), UseTables::EntryChange());

          String afterClause;
          String orderClause = " ORDER BY " + String(SqlField::id) + " ASC";
          if (OPENPEER_STACK_LOCATION_DATABASE_INDEX_UNKNOWN != afterIndexEntryChanged) {
            afterClause = String(SqlField::id) + " > " + string(afterIndexEntryChanged);
          } else {
            afterClause = String(SqlField::id) + " > " + string(0);
          }

          table.open(afterClause + orderClause + " LIMIT " + string(OPENPEER_STACK_SERVICES_LOCATION_DATABASE_ABSTRACTION_ENTRY_CHANGE_BATCH_LIMIT));

          for (int row = 0; row < table.recordCount(); ++row) {
            SqlRecord *record = table.getRecord(row);

            EntryChangeRecord entryChangeRecord;
            internal::convert(record, entryChangeRecord);

            ZS_LOG_TRACE(log("found entry change record") + entryChangeRecord.toDebug())

            result->push_back(entryChangeRecord);
          }

          return result;
        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database failure") + ZS_PARAM("message", e.msg()))
        }

        return EntryChangeRecordListPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => SqlDatabase::Trace
      #pragma mark

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::notifyDatabaseTrace(
                                                            SqlDatabase::Trace::Severity severity,
                                                            const char *message
                                                            ) const
      {
        switch (severity) {
          case SqlDatabase::Trace::Informational: ZS_LOG_TRACE_WITH_SEVERITY(UseStackHelper::toSeverity(severity), log(message)) break;
          case SqlDatabase::Trace::Warning:       ZS_LOG_DETAIL_WITH_SEVERITY(UseStackHelper::toSeverity(severity), log(message)) break;
          case SqlDatabase::Trace::Error:         ZS_LOG_DETAIL_WITH_SEVERITY(UseStackHelper::toSeverity(severity), log(message)) break;
          case SqlDatabase::Trace::Fatal:         ZS_LOG_BASIC_WITH_SEVERITY(UseStackHelper::toSeverity(severity), log(message)) break;
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LocationDatabaseAbstraction => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params LocationDatabaseAbstraction::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("LocationDatabaseAbstraction");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      Log::Params LocationDatabaseAbstraction::debug(const char *message) const
      {
        return Log::Params(message, toDebug());
      }

      //-----------------------------------------------------------------------
      ElementPtr LocationDatabaseAbstraction::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("LocationDatabaseAbstraction");

        UseServicesHelper::debugAppend(resultEl, "id", mID);

        UseServicesHelper::debugAppend(resultEl, "user hash", mUserHash);
        UseServicesHelper::debugAppend(resultEl, "storage path", mStoragePath);

        UseServicesHelper::debugAppend(resultEl, "database", (bool)mDB);

        UseServicesHelper::debugAppend(resultEl, "master", mMaster ? mMaster->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "peer uri", mPeerURI);
        UseServicesHelper::debugAppend(resultEl, "location id", mLocationID);
        UseServicesHelper::debugAppend(resultEl, "database id", mDatabaseID);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::cancel()
      {
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::prepareDB()
      {
        if (mDB) return;

        bool alreadyDeleted = false;

        if (!UseStackHelper::mkdir(mStoragePath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
          ZS_LOG_ERROR(Detail, log("unable to create storage path") + ZS_PARAMIZE(mStoragePath))
          return;
        }

        String userPath = UseStackHelper::appendPath(mStoragePath, mUserHash);

        ZS_THROW_BAD_STATE_IF(userPath.isEmpty())

        if (!UseStackHelper::mkdir(userPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
          ZS_LOG_ERROR(Detail, log("unable to create storage path") + ZS_PARAMIZE(userPath))
          return;
        }

        String dbFileName;
        if (mMaster) {
          String locationHash = UseServicesHelper::convertToHex(*UseServicesHelper::hash(mPeerURI + ":" + mLocationID));
          String dbHash = UseServicesHelper::convertToHex(*UseServicesHelper::hash(mLocationID));
          dbFileName = UseSettings::getString(OPENPEER_STACK_SETTING_LOCATION_DATABASE_FILE_PREFIX) +locationHash + "_" + dbHash + UseSettings::getString(OPENPEER_STACK_SETTING_LOCATION_DATABASE_FILE_POSTFIX);
        } else {
          dbFileName = UseSettings::getString(OPENPEER_STACK_SETTING_LOCATION_MASTER_DATABASE_FILE);
        }

        String path = UseStackHelper::appendPath(userPath, dbFileName);
        ZS_THROW_BAD_STATE_IF(path.isEmpty())

        try {
          while (true) {
            ZS_LOG_DETAIL(log("loading location database") + ZS_PARAM("path", path))

            mDB = SqlDatabasePtr(new SqlDatabase(ZS_IS_LOGGING(Trace) ? this : NULL));

            mDB->open(path);

            SqlTable versionTable(*mDB, UseTables::Version_name(), UseTables::Version());

            if (!versionTable.exists()) {
              versionTable.create();

              SqlRecord record(versionTable.fields());
              record.setInteger(UseTables::version, OPENPEER_STACK_LOCATION_DATABASE_VERSION);
              versionTable.addRecord(&record);

              constructDBTables();
            }

            versionTable.open();

            SqlRecord *versionRecord = versionTable.getTopRecord();

            auto databaseVersion = versionRecord->getValue(UseTables::version)->asInteger();

            switch (databaseVersion) {
              // "upgrades" are possible here...
              case OPENPEER_STACK_LOCATION_DATABASE_VERSION: {
                ZS_LOG_DEBUG(log("location database loaded"))
                auto randomlyAnalyze = UseSettings::getUInt(OPENPEER_STACK_SETTING_LOCATION_ANALYZE_DATABASE_RANDOMLY_EVERY);
                if (0 != randomlyAnalyze) {
                  if (0 == UseServicesHelper::random(0, randomlyAnalyze)) {
                    ZS_LOG_BASIC(log("performing random database analyzation") + ZS_PARAM(OPENPEER_STACK_SETTING_LOCATION_ANALYZE_DATABASE_RANDOMLY_EVERY, randomlyAnalyze))
                    SqlRecordSet query(*mDB);
                    query.query("ANALYZE");
                  }
                }
                goto database_open_complete;
              }
              default: {
                ZS_LOG_DEBUG(log("location database version is not not compatible (thus will delete database)"))

                // close the database
                mDB.reset();

                if (alreadyDeleted) {
                  // already attempted delete once
                  ZS_LOG_FATAL(Basic, log("failed to load database"))
                  goto database_open_complete;
                }

                auto status = remove(path);
                if (0 != status) {
                  ZS_LOG_ERROR(Detail, log("attempt to remove incompatible location database file failed") + ZS_PARAM("path", path) + ZS_PARAM("result", status) + ZS_PARAM("errno", errno))
                }
                alreadyDeleted = true;
                break;
              }
            }
          }

        } catch (SqlException &e) {
          ZS_LOG_ERROR(Detail, log("database preparation failure") + ZS_PARAM("message", e.msg()))
          mDB.reset();
        }

      database_open_complete:
        {
        }
      }

      //-----------------------------------------------------------------------
      void LocationDatabaseAbstraction::constructDBTables()
      {
        ZS_LOG_DEBUG(log("constructing location database tables"))

        if (!mMaster) {
          {
            SqlTable table(*mDB, UseTables::PeerLocation_name(), UseTables::PeerLocation());
            table.create();

            SqlRecordSet query(*mDB);
            query.query(String("CREATE INDEX ") + UseTables::PeerLocation_name() + "i" + UseTables::downloadComplete + " ON " + UseTables::PeerLocation_name() + "(" + UseTables::downloadComplete + ")");
            query.query(String("CREATE INDEX ") + UseTables::PeerLocation_name() + "i" + UseTables::notified + " ON " + UseTables::PeerLocation_name() + "(" + UseTables::notified + ")");
            query.query(String("CREATE INDEX ") + UseTables::PeerLocation_name() + "i" + UseTables::lastAccessed + " ON " + UseTables::PeerLocation_name() + "(" + UseTables::lastAccessed + ")");
          }
          {
            SqlTable table(*mDB, UseTables::Database_name(), UseTables::Database());
            table.create();

            SqlRecordSet query(*mDB);
            query.query(String("CREATE INDEX ") + UseTables::Database_name() + "i" + UseTables::indexPeerLocation + " ON " + UseTables::Database_name() + "(" + UseTables::indexPeerLocation + ")");
            query.query(String("CREATE INDEX ") + UseTables::Database_name() + "i" + UseTables::databaseID + " ON " + UseTables::Database_name() + "(" + UseTables::databaseID + ")");
          }

          {
            SqlTable table(*mDB, UseTables::DatabaseChange_name(), UseTables::DatabaseChange());
            table.create();

            SqlRecordSet query(*mDB);
            query.query(String("CREATE INDEX ") + UseTables::DatabaseChange_name() + "i" + UseTables::indexPeerLocation + " ON " + UseTables::DatabaseChange_name() + "(" + UseTables::indexPeerLocation + ")");
            query.query(String("CREATE INDEX ") + UseTables::DatabaseChange_name() + "i" + UseTables::indexDatabase + " ON " + UseTables::DatabaseChange_name() + "(" + UseTables::indexDatabase + ")");
          }
          {
            SqlTable table(*mDB, UseTables::Permission_name(), UseTables::Permission());
            table.create();

            SqlRecordSet query(*mDB);
            query.query(String("CREATE INDEX ") + UseTables::Permission_name() + "i" + UseTables::peerURI + " ON " + UseTables::Permission_name() + "(" + UseTables::peerURI + ")");
            query.query(String("CREATE INDEX ") + UseTables::Permission_name() + "i" + UseTables::indexPeerLocation + " ON " + UseTables::Permission_name() + "(" + UseTables::indexPeerLocation + ")");
            query.query(String("CREATE INDEX ") + UseTables::Permission_name() + "i" + UseTables::indexDatabase + " ON " + UseTables::Permission_name() + "(" + UseTables::indexDatabase + ")");
          }
        }
        else
        {
          {
            SqlTable table(*mDB, UseTables::Entry_name(), UseTables::Entry());
            table.create();
          }
          {
            SqlTable table(*mDB, UseTables::EntryChange_name(), UseTables::EntryChange());
            table.create();
          }
        }
      }

      //-----------------------------------------------------------------------
      String LocationDatabaseAbstraction::SqlEscape(const String &input)
      {
        String result(input);
        result.replaceAll("\'", "\'\'");
        return result;
      }

      //-----------------------------------------------------------------------
      String LocationDatabaseAbstraction::SqlQuote(const String &input)
      {
        String result(input);
        result.replaceAll("\'", "\'\'");
        return "\'" + result + "\'";
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ILocationDatabaseAbstractionFactory
      #pragma mark

      //-----------------------------------------------------------------------
      ILocationDatabaseAbstractionFactory &ILocationDatabaseAbstractionFactory::singleton()
      {
        return LocationDatabaseAbstractionFactory::singleton();
      }

      //-----------------------------------------------------------------------
      LocationDatabaseAbstractionPtr ILocationDatabaseAbstractionFactory::createMaster(
                                                                                       const char *inHashRepresentingUser,
                                                                                       const char *inUserStorageFilePath
                                                                                       )
      {
        if (this) {}
        return LocationDatabaseAbstraction::createMaster(inHashRepresentingUser, inUserStorageFilePath);
      }

      //-----------------------------------------------------------------------
      LocationDatabaseAbstractionPtr ILocationDatabaseAbstractionFactory::openDatabase(
                                                                                       ILocationDatabaseAbstractionPtr masterDatabase,
                                                                                       const char *peerURI,
                                                                                       const char *locationID,
                                                                                       const char *databaseID
                                                                                       )
      {
        if (this) {}
        return LocationDatabaseAbstraction::openDatabase(masterDatabase, peerURI, locationID, databaseID);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ILocationDatabaseAbstraction
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr ILocationDatabaseAbstraction::toDebug(ILocationDatabaseAbstractionPtr session)
      {
        return LocationDatabaseAbstraction::toDebug(session);
      }

      //-----------------------------------------------------------------------
      ILocationDatabaseAbstractionPtr ILocationDatabaseAbstraction::createMaster(
                                                                                 const char *inHashRepresentingUser,
                                                                                 const char *inUserStorageFilePath
                                                                                 )
      {
        return ILocationDatabaseAbstractionFactory::singleton().createMaster(inHashRepresentingUser, inUserStorageFilePath);
      }

      //-----------------------------------------------------------------------
      ILocationDatabaseAbstractionPtr ILocationDatabaseAbstraction::openDatabase(
                                                                                 ILocationDatabaseAbstractionPtr masterDatabase,
                                                                                 const char *peerURI,
                                                                                 const char *locationID,
                                                                                 const char *databaseID
                                                                                 )
      {
        return ILocationDatabaseAbstractionFactory::singleton().openDatabase(masterDatabase, peerURI, locationID, databaseID);
      }

    }

  }
}
