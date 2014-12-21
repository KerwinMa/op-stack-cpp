/*

 Copyright (c) 2013, SMB Phone Inc.
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

#include "TestPushMailboxDB.h"
#include "TestSetup.h"

#include <zsLib/MessageQueueThread.h>
#include <zsLib/Exception.h>
#include <zsLib/Proxy.h>

#include <openpeer/stack/IStack.h>
#include <openpeer/stack/ICache.h>
#include <openpeer/stack/ISettings.h>

#include <openpeer/services/IHelper.h>

#include "config.h"
#include "testing.h"

#include <list>
#include <sys/stat.h>

using zsLib::ULONG;

ZS_DECLARE_USING_PTR(openpeer::stack, IStack)
ZS_DECLARE_USING_PTR(openpeer::stack, ICache)
ZS_DECLARE_USING_PTR(openpeer::stack, ISettings)

ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

ZS_DECLARE_USING_PTR(openpeer::stack::test, TestPushMailboxDB)
ZS_DECLARE_USING_PTR(openpeer::stack::test, TestSetup)

ZS_DECLARE_TYPEDEF_PTR(sql::Value, SqlValue)

ZS_DECLARE_TYPEDEF_PTR(openpeer::stack::internal::IServicePushMailboxSessionDatabaseAbstractionFactory, IUseAbstractionFactory)
ZS_DECLARE_TYPEDEF_PTR(openpeer::stack::internal::ServicePushMailboxSessionDatabaseAbstractionFactory, UseAbstractionFactory)


namespace openpeer { namespace stack { namespace test { ZS_DECLARE_SUBSYSTEM(openpeer_stack_test) } } }

namespace openpeer
{
  namespace stack
  {
    namespace test
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark OverrideAbstractionFactory
      #pragma mark

      interaction OverrideAbstractionFactory : public IUseAbstractionFactory
      {
        ZS_DECLARE_TYPEDEF_PTR(openpeer::stack::internal::ServicePushMailboxSessionDatabaseAbstraction, UseAbstraction)

        static OverrideAbstractionFactoryPtr create()
        {
          return OverrideAbstractionFactoryPtr(new OverrideAbstractionFactory);
        }

        virtual UseAbstractionPtr create(
                                         const char *inHashRepresentingUser,
                                         const char *inUserTemporaryFilePath,
                                         const char *inUserStorageFilePath
                                         )
        {
          return OverridePushMailboxAbstraction::create(inHashRepresentingUser, inUserTemporaryFilePath, inUserStorageFilePath);
        }

      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark OverridePushMailboxAbstraction
      #pragma mark

      //-----------------------------------------------------------------------
      OverridePushMailboxAbstraction::OverridePushMailboxAbstraction(
                                                                     const char *inHashRepresentingUser,
                                                                     const char *inUserTemporaryFilePath,
                                                                     const char *inUserStorageFilePath
                                                                     ) :
        UseAbstraction(inHashRepresentingUser, inUserTemporaryFilePath, inUserStorageFilePath)
      {
      }

      //-----------------------------------------------------------------------
      OverridePushMailboxAbstractionPtr OverridePushMailboxAbstraction::convert(IUseAbstractionPtr abstraction)
      {
        return ZS_DYNAMIC_PTR_CAST(OverridePushMailboxAbstraction, abstraction);
      }

      //-----------------------------------------------------------------------
      OverridePushMailboxAbstractionPtr OverridePushMailboxAbstraction::create(
                                                                               const char *inHashRepresentingUser,
                                                                               const char *inUserTemporaryFilePath,
                                                                               const char *inUserStorageFilePath
                                                                               )
      {
        OverridePushMailboxAbstractionPtr pThis(new OverridePushMailboxAbstraction(inHashRepresentingUser, inUserTemporaryFilePath, inUserStorageFilePath));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void OverridePushMailboxAbstraction::init()
      {
        UseAbstraction::init();
      }

      //-----------------------------------------------------------------------
      OverridePushMailboxAbstraction::SqlDatabasePtr OverridePushMailboxAbstraction::getDatabase()
      {
        return mDB;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark TestPushMailboxDB
      #pragma mark

      //-----------------------------------------------------------------------
      TestPushMailboxDB::TestPushMailboxDB() :
        mUserHash(UseServicesHelper::randomString(8))
      {
        mFactory = OverrideAbstractionFactory::create();

        UseAbstractionFactory::override(mFactory);
      }

      //-----------------------------------------------------------------------
      TestPushMailboxDB::~TestPushMailboxDB()
      {
        mThisWeak.reset();

        UseAbstractionFactory::override(UseAbstractionFactoryPtr());
      }

      //-----------------------------------------------------------------------
      TestPushMailboxDBPtr TestPushMailboxDB::create()
      {
        TestPushMailboxDBPtr pThis(new TestPushMailboxDB());
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::init()
      {
        ISettings::applyDefaults();
        ISettings::setString("openpeer/stack/cache-database-path", "/tmp");
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::testCreate()
      {
        mkdir("/tmp/testpushdb", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir("/tmp/testpushdb/tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        mkdir("/tmp/testpushdb/users", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        mAbstraction = IUseAbstraction::create(mUserHash, "/tmp/testpushdb/tmp", "/tmp/testpushdb/users");
        mOverride = OverridePushMailboxAbstraction::convert(mAbstraction);

        TESTING_CHECK(mAbstraction)
        TESTING_CHECK(mOverride)

        checkIndexValue(UseTables::Version(), UseTables::Version_name(), 0, UseTables::version, 1);
        checkIndexValue(UseTables::Settings(), UseTables::Settings_name(), 0, UseTables::lastDownloadedVersionForFolders, String());
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::testSettings()
      {
        auto settings = mAbstraction->settingsTable();
        TESTING_CHECK(settings)

        {
          String result = settings->getLastDownloadedVersionForFolders();
          TESTING_EQUAL(result, String())
          checkIndexValue(UseTables::Settings(), UseTables::Settings_name(), 0, UseTables::lastDownloadedVersionForFolders, String());
        }
        {
          String value("foo-1");
          settings->setLastDownloadedVersionForFolders(value);
          String result = settings->getLastDownloadedVersionForFolders();
          TESTING_EQUAL(result, value)
          checkIndexValue(UseTables::Settings(), UseTables::Settings_name(), 0, UseTables::lastDownloadedVersionForFolders, value);
        }
        {
          String value("foo-2");
          settings->setLastDownloadedVersionForFolders(value);
          String result = settings->getLastDownloadedVersionForFolders();
          TESTING_EQUAL(result, value)
          checkIndexValue(UseTables::Settings(), UseTables::Settings_name(), 0, UseTables::lastDownloadedVersionForFolders, value);
        }
        {
          String value;
          settings->setLastDownloadedVersionForFolders(value);
          String result = settings->getLastDownloadedVersionForFolders();
          TESTING_EQUAL(result, value)
          checkIndexValue(UseTables::Settings(), UseTables::Settings_name(), 0, UseTables::lastDownloadedVersionForFolders, value);
        }
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::testFolder()
      {
        auto folder = mAbstraction->folderTable();
        TESTING_CHECK(folder)

        {
          folder->resetUniqueID(5); // this will not do anything
        }

        String originalUniqueID;

        {
          IUseAbstraction::FolderRecord value;
          value.mFolderName = "inbox";
          value.mTotalUnreadMessages = 5;
          value.mTotalMessages = 7;
          value.mUpdateNext = zsLib::now() + Seconds(5);
          folder->addOrUpdate(value);

          auto index = folder->getIndex("inbox");
          TESTING_EQUAL(index, 1)

          auto resultRecord = folder->getIndexAndUniqueID("inbox");
          TESTING_CHECK(resultRecord)
          TESTING_EQUAL(resultRecord->mIndex, 1)
          TESTING_EQUAL(resultRecord->mFolderName, "inbox")
          TESTING_CHECK(resultRecord->mUniqueID.hasData())
          TESTING_EQUAL(resultRecord->mServerVersion, String())
          TESTING_EQUAL(resultRecord->mDownloadedVersion, String())
          TESTING_EQUAL(resultRecord->mTotalUnreadMessages, 5)
          TESTING_EQUAL(resultRecord->mTotalMessages, 7)
          TESTING_CHECK(zsLib::now() < resultRecord->mUpdateNext)
          TESTING_CHECK(zsLib::now() + Seconds(10) > resultRecord->mUpdateNext)

          originalUniqueID = resultRecord->mUniqueID;

          auto name = folder->getName(index);
          TESTING_EQUAL(name, "inbox")

          folder->updateFolderName(index, "inbox2");

          auto name2 = folder->getName(index);
          TESTING_EQUAL(name2, "inbox2")
        }

        {
          folder->updateServerVersionIfFolderExists("inbox2", "server-1");

          auto resultRecord = folder->getIndexAndUniqueID("inbox2");
          TESTING_EQUAL(resultRecord->mIndex, 1)
          TESTING_EQUAL(resultRecord->mFolderName, "inbox2")
          TESTING_CHECK(resultRecord->mUniqueID.hasData())
          TESTING_EQUAL(resultRecord->mServerVersion, "server-1")
          TESTING_EQUAL(resultRecord->mDownloadedVersion, String())
          TESTING_EQUAL(resultRecord->mTotalUnreadMessages, 5)
          TESTING_EQUAL(resultRecord->mTotalMessages, 7)
          TESTING_CHECK(zsLib::now() < resultRecord->mUpdateNext)
          TESTING_CHECK(zsLib::now() + Seconds(10) > resultRecord->mUpdateNext)
        }

        {
          folder->updateServerVersionIfFolderExists("bogus", "server-2"); // noop
        }

        {
          folder->updateDownloadInfo(1, "download-1", zsLib::now() + Seconds(15));
        }

        {
          folder->resetUniqueID(1);

          auto resultRecord = folder->getIndexAndUniqueID("inbox2");
          TESTING_EQUAL(resultRecord->mIndex, 1)
          TESTING_EQUAL(resultRecord->mFolderName, "inbox2")
          TESTING_CHECK(resultRecord->mUniqueID.hasData())
          TESTING_CHECK(resultRecord->mUniqueID != originalUniqueID)
          TESTING_CHECK(resultRecord->mUniqueID.hasData())
          TESTING_EQUAL(resultRecord->mServerVersion, "server-1")
          TESTING_EQUAL(resultRecord->mDownloadedVersion, "download-1")
          TESTING_EQUAL(resultRecord->mTotalUnreadMessages, 5)
          TESTING_EQUAL(resultRecord->mTotalMessages, 7)
          TESTING_CHECK(zsLib::now() < resultRecord->mUpdateNext)
          TESTING_CHECK(zsLib::now() + Seconds(10) < resultRecord->mUpdateNext)
        }

      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::testFolderMessage()
      {
        auto folderMessage = mAbstraction->folderMessageTable();
        TESTING_CHECK(folderMessage)

        {
          folderMessage->flushAll();            // nooop
        }
        
        {
          IUseAbstraction::FolderMessageRecord record;
          record.mIndexFolderRecord = 5;
          record.mMessageID = "foo";
          folderMessage->addToFolderIfNotPresent(record);

          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, SqlField::id, 1);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, UseTables::indexFolderRecord, 5);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, UseTables::messageID, "foo");
        }
        
        {
          IUseAbstraction::FolderMessageRecord record;
          record.mIndexFolderRecord = 6;
          record.mMessageID = "foo";
          folderMessage->addToFolderIfNotPresent(record);

          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, SqlField::id, 2);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, UseTables::indexFolderRecord, 6);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, UseTables::messageID, "foo");
        }

        {
          IUseAbstraction::FolderMessageRecord record;      // noop
          record.mIndexFolderRecord = 5;
          record.mMessageID = "foo";
          folderMessage->addToFolderIfNotPresent(record);
        }

        {
          IUseAbstraction::FolderMessageRecord record;
          record.mIndexFolderRecord = 4;
          record.mMessageID = "bar";
          folderMessage->addToFolderIfNotPresent(record);

          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 2, SqlField::id, 3);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 2, UseTables::indexFolderRecord, 4);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 2, UseTables::messageID, "bar");
        }

        {
          IUseAbstraction::FolderMessageRecord record;
          record.mIndexFolderRecord = 6;
          record.mMessageID = "bar";
          folderMessage->addToFolderIfNotPresent(record);

          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 3, SqlField::id, 4);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 3, UseTables::indexFolderRecord, 6);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 3, UseTables::messageID, "bar");
        }
        
        checkCount(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 4);
        
        {
          IUseAbstraction::IFolderMessageTable::IndexListPtr indexes = folderMessage->getWithMessageID("foo");

          int loop = 0;
          for (auto iter = indexes->begin(); iter != indexes->end(); ++iter, ++loop)
          {
            auto value = (*iter);
            switch (loop) {
              case 0:   TESTING_EQUAL(5, value); break;
              case 1:   TESTING_EQUAL(6, value); break;
              default:  TESTING_CHECK(false); break;
            }
          }
          TESTING_EQUAL(loop, 2)
        }

        {
          folderMessage->flushAll();            // remove all four
          checkCount(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0);
        }
        
        {
          IUseAbstraction::FolderMessageRecord record;
          record.mIndexFolderRecord = 9;
          record.mMessageID = "foo2";
          folderMessage->addToFolderIfNotPresent(record);
          
          checkCount(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, SqlField::id, 1);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, UseTables::indexFolderRecord, 9);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, UseTables::messageID, "foo2");
        }
        {
          IUseAbstraction::FolderMessageRecord record;
          record.mIndexFolderRecord = 8;
          record.mMessageID = "foo2";
          folderMessage->addToFolderIfNotPresent(record);
          
          checkCount(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 2);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, SqlField::id, 2);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, UseTables::indexFolderRecord, 8);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, UseTables::messageID, "foo2");
        }
        {
          IUseAbstraction::FolderMessageRecord record;
          record.mIndexFolderRecord = 9;
          record.mMessageID = "foo";
          folderMessage->addToFolderIfNotPresent(record);
          
          checkCount(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 3);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 2, SqlField::id, 3);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 2, UseTables::indexFolderRecord, 9);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 2, UseTables::messageID, "foo");
        }
        
        {
          folderMessage->removeAllFromFolder(9);
          checkCount(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, SqlField::id, 2);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, UseTables::indexFolderRecord, 8);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 0, UseTables::messageID, "foo2");
        }
        {
          IUseAbstraction::FolderMessageRecord record;
          record.mIndexFolderRecord = 9;
          record.mMessageID = "foo";
          folderMessage->addToFolderIfNotPresent(record);
          
          checkCount(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 2);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, SqlField::id, 3);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, UseTables::indexFolderRecord, 9);
          checkIndexValue(UseTables::FolderMessage(), UseTables::FolderMessage_name(), 1, UseTables::messageID, "foo");
        }
      }
      
      //-----------------------------------------------------------------------
      void TestPushMailboxDB::testFolderVersionedMessage()
      {
        auto folderVersionedMessage = mAbstraction->folderVersionedMessageTable();
        TESTING_CHECK(folderVersionedMessage)

        {
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0);
          folderVersionedMessage->flushAll();   // noop
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0);
        }

        {
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 5;
          record.mMessageID = "foo";
          record.mRemovedFlag = false;

          folderVersionedMessage->add(record);
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0, SqlField::id, 1);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0, UseTables::indexFolderRecord, 5);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0, UseTables::messageID, "foo");
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0, UseTables::removedFlag, 0);
        }

        {
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 5;
          record.mMessageID = "foo";
          
          folderVersionedMessage->addRemovedEntryIfNotAlreadyRemoved(record);
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1, SqlField::id, 2);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1, UseTables::indexFolderRecord, 5);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1, UseTables::messageID, "foo");
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1, UseTables::removedFlag, 1);
        }

        {
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 6;
          record.mMessageID = "foo2";
          
          folderVersionedMessage->add(record);   // noop
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 3);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, SqlField::id, 3);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::indexFolderRecord, 6);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::messageID, "foo2");
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::removedFlag, 0);
        }
        
        {
          folderVersionedMessage->removeAllRelatedToFolder(6);
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2);
          
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 6;
          record.mMessageID = "foo2";
          folderVersionedMessage->addRemovedEntryIfNotAlreadyRemoved(record); // noop
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2);
        }
        {
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 8;
          record.mMessageID = "foo3";
          
          folderVersionedMessage->add(record);   // noop

          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 3);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, SqlField::id, 4);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::indexFolderRecord, 8);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::messageID, "foo3");
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::removedFlag, 0);
        }

        {
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 3);
          folderVersionedMessage->flushAll();   // noop
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0);
        }

        {
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 10;
          record.mMessageID = "foo4";
          
          folderVersionedMessage->add(record);   // noop
          
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0, SqlField::id, 5);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0, UseTables::indexFolderRecord, 10);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0, UseTables::messageID, "foo4");
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 0, UseTables::removedFlag, 0);
        }
        {
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 10;
          record.mMessageID = "foo5";
          
          folderVersionedMessage->add(record);   // noop
          
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1, SqlField::id, 6);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1, UseTables::indexFolderRecord, 10);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1, UseTables::messageID, "foo5");
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 1, UseTables::removedFlag, 0);
        }
        {
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 8;
          record.mMessageID = "foo6";
          
          folderVersionedMessage->add(record);   // noop
          
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 3);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, SqlField::id, 7);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::indexFolderRecord, 8);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::messageID, "foo6");
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 2, UseTables::removedFlag, 0);
        }
        {
          IUseAbstraction::FolderVersionedMessageRecord record;
          record.mIndexFolderRecord = 8;
          record.mMessageID = "foo6";
          
          folderVersionedMessage->addRemovedEntryIfNotAlreadyRemoved(record);   // noop
          
          checkCount(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 4);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 3, SqlField::id, 8);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 3, UseTables::indexFolderRecord, 8);
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 3, UseTables::messageID, "foo6");
          checkIndexValue(UseTables::FolderVersionedMessage(), UseTables::FolderVersionedMessage_name(), 3, UseTables::removedFlag, 1);
        }
        
        {
          auto result = folderVersionedMessage->getBatchAfterIndex(5, 10);
          int loop = 0;
          for (auto iter = result->begin(); iter != result->end(); ++loop, ++iter)
          {
            auto value = (*iter);
            switch (loop) {
              case 0: {
                TESTING_EQUAL(value.mIndex, 6)
                TESTING_EQUAL(value.mIndexFolderRecord, 10)
                TESTING_EQUAL(value.mMessageID, "foo5")
                TESTING_CHECK(!value.mRemovedFlag)
                break;
              }
              default: TESTING_CHECK(false); break;
            }
          }
          TESTING_EQUAL(1, loop)
        }

        {
          auto result = folderVersionedMessage->getBatchAfterIndex(-1, 8);
          int loop = 0;
          for (auto iter = result->begin(); iter != result->end(); ++loop, ++iter)
          {
            auto value = (*iter);
            switch (loop) {
              case 0: {
                TESTING_EQUAL(value.mIndex, 7)
                TESTING_EQUAL(value.mIndexFolderRecord, 8)
                TESTING_EQUAL(value.mMessageID, "foo6")
                TESTING_CHECK(!value.mRemovedFlag)
                break;
              }
              case 1: {
                TESTING_EQUAL(value.mIndex, 8)
                TESTING_EQUAL(value.mIndexFolderRecord, 8)
                TESTING_EQUAL(value.mMessageID, "foo6")
                TESTING_CHECK(value.mRemovedFlag)
                break;
              }
              default: TESTING_CHECK(false); break;
            }
          }
          TESTING_EQUAL(2, loop)
        }
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::testMessage()
      {
        auto message = mAbstraction->messageTable();
        TESTING_CHECK(message)
        
        {
          message->addOrUpdate("foo", "v1");
          checkCount(UseTables::Message(), UseTables::Message_name(), 1);
          checkIndexValue(UseTables::Message(), UseTables::Message_name(), 0, SqlField::id, 1);
          checkIndexValue(UseTables::Message(), UseTables::Message_name(), 0, UseTables::messageID, "foo");
          checkIndexValue(UseTables::Message(), UseTables::Message_name(), 0, UseTables::serverVersion, "v1");
        }

        {
          message->addOrUpdate("foo", "v2");
          checkCount(UseTables::Message(), UseTables::Message_name(), 1);
          checkIndexValue(UseTables::Message(), UseTables::Message_name(), 0, SqlField::id, 1);
          checkIndexValue(UseTables::Message(), UseTables::Message_name(), 0, UseTables::messageID, "foo");
          checkIndexValue(UseTables::Message(), UseTables::Message_name(), 0, UseTables::serverVersion, "v2");
        }
      }
      
      //-----------------------------------------------------------------------
      void TestPushMailboxDB::checkCount(
                                         SqlField *definition,
                                         const char *tableName,
                                         int total
                                         )
      {
        TESTING_CHECK(definition)
        TESTING_CHECK(tableName)
        
        TESTING_CHECK(mOverride)
        if (!mOverride) return;
        
        SqlDatabasePtr database = mOverride->getDatabase();
        TESTING_CHECK(database)
        if (!database) return;
        
        SqlTable table(database->getHandle(), tableName, definition);
        table.open();
        
        TESTING_EQUAL(table.recordCount(), total);
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::checkIndexValue(
                                              SqlField *definition,
                                              const char *tableName,
                                              int index,
                                              const char *fieldName,
                                              const char *value
                                              )
      {
        TESTING_CHECK(definition)
        TESTING_CHECK(tableName)

        TESTING_CHECK(mOverride)
        if (!mOverride) return;

        SqlDatabasePtr database = mOverride->getDatabase();
        TESTING_CHECK(database)
        if (!database) return;

        SqlTable table(database->getHandle(), tableName, definition);
        table.open();
        checkIndexValue(&table, index, fieldName, value);
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::checkIndexValue(
                                              SqlField *definition,
                                              const char *tableName,
                                              int index,
                                              const char *fieldName,
                                              int value
                                              )
      {
        TESTING_CHECK(definition)
        TESTING_CHECK(tableName)

        TESTING_CHECK(mOverride)
        if (!mOverride) return;

        SqlDatabasePtr database = mOverride->getDatabase();
        TESTING_CHECK(database)
        if (!database) return;

        SqlTable table(database->getHandle(), tableName, definition);
        table.open();
        checkIndexValue(&table, index, fieldName, value);
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::checkIndexValue(
                                              SqlTable *table,
                                              int index,
                                              const char *fieldName,
                                              const char *value
                                              )
      {
        TESTING_CHECK(table)

        SqlRecord *record = table->getRecord(index);

        checkValue(record, fieldName, value);
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::checkIndexValue(
                                              SqlTable *table,
                                              int index,
                                              const char *fieldName,
                                              int value
                                              )
      {
        TESTING_CHECK(table)

        SqlRecord *record = table->getRecord(index);

        checkValue(record, fieldName, value);
      }
      
      //-----------------------------------------------------------------------
      void TestPushMailboxDB::checkValue(
                                         SqlRecord *record,
                                         const char *fieldName,
                                         const char *inValue
                                         )
      {
        TESTING_CHECK(record)
        if (!record) return;

        SqlValue *value = record->getValue(fieldName);
        TESTING_CHECK(value)

        if (!value) return;

        String result = value->asString();

        TESTING_EQUAL(result, String(inValue))
      }

      //-----------------------------------------------------------------------
      void TestPushMailboxDB::checkValue(
                                         SqlRecord *record,
                                         const char *fieldName,
                                         int inValue
                                         )
      {
        TESTING_CHECK(record)
        if (!record) return;

        SqlValue *value = record->getValue(fieldName);
        TESTING_CHECK(value)

        if (!value) return;

        auto result = value->asInteger();

        TESTING_EQUAL(result, inValue)
      }
    }
  }
}

void doTestPushMailboxDB()
{
  if (!OPENPEER_STACK_TEST_DO_PUSH_MAILBOX_DB_TEST) return;

  TESTING_INSTALL_LOGGER();

  TestSetupPtr setup = TestSetup::singleton();

  TestPushMailboxDBPtr testObject = TestPushMailboxDB::create();

  testObject->testCreate();
  testObject->testSettings();
  testObject->testFolder();
  testObject->testFolderMessage();
  testObject->testFolderVersionedMessage();
  testObject->testMessage();

  std::cout << "COMPLETE:     Push mailbox db complete.\n";

//  TESTING_CHECK(testObject->mNetworkDone)
//  TESTING_CHECK(testObject->mNetwork->isPreparationComplete())
//  TESTING_CHECK(testObject->mNetwork->wasSuccessful())


  TESTING_UNINSTALL_LOGGER()
}