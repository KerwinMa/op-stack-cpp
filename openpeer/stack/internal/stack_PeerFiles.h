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

#pragma once

#include <openpeer/stack/IPeerFiles.h>
#include <openpeer/stack/internal/types.h>

namespace openpeer
{
  namespace stack
  {
    namespace internal
    {
      interaction IPeerFilePrivateForPeerFiles;
      interaction IPeerFilePublicForPeerFiles;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PeerFiles
      #pragma mark

      class PeerFiles : public Noop,
                        public IPeerFiles
      {
      public:
        friend interaction IPeerFilesFactory;
        friend interaction IPeerFiles;

        typedef IPeerFilePrivateForPeerFiles UsePeerFilePrivate;
        typedef shared_ptr<UsePeerFilePrivate> UsePeerFilePrivatePtr;
        typedef weak_ptr<UsePeerFilePrivate> UsePeerFilePrivateWeakPtr;

        typedef IPeerFilePublicForPeerFiles UsePeerFilePublic;
        typedef shared_ptr<UsePeerFilePublic> UsePeerFilePublicPtr;
        typedef weak_ptr<UsePeerFilePublic> UsePeerFilePublicWeakPtr;

      protected:
        PeerFiles();
        
        PeerFiles(Noop) : Noop(true) {};

      public:
        ~PeerFiles();

        static PeerFilesPtr convert(IPeerFilesPtr object);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerFiles => IPeerFiles
        #pragma mark

        static ElementPtr toDebug(IPeerFilesPtr peerFiles);

        static PeerFilesPtr generate(
                                     const char *password,
                                     ElementPtr signedSaltBundleEl
                                     );

        static PeerFilesPtr generate(
                                     IRSAPrivateKeyPtr privateKey,
                                     IRSAPublicKeyPtr publicKey,
                                     const char *password,
                                     ElementPtr signedSaltBundleEl
                                     );

        static PeerFilesPtr loadFromElement(
                                            const char *password,
                                            ElementPtr privatePeerRootElement
                                            );

        virtual PUID getID() const {return mID;}

        virtual ElementPtr saveToPrivatePeerElement() const;

        virtual IPeerFilePublicPtr getPeerFilePublic() const;
        virtual IPeerFilePrivatePtr getPeerFilePrivate() const;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerFiles => (internal)
        #pragma mark

        Log::Params log(const char *message) const;

        virtual ElementPtr toDebug() const;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark PeerFiles => (data)
        #pragma mark

        PUID mID;
        mutable RecursiveLock mLock;
        PeerFilesWeakPtr mThisWeak;

        UsePeerFilePublicPtr mPublic;
        UsePeerFilePrivatePtr mPrivate;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IPeerFilesFactory
      #pragma mark

      interaction IPeerFilesFactory
      {
        static IPeerFilesFactory &singleton();

        virtual PeerFilesPtr generate(
                                      const char *password,
                                      ElementPtr signedSaltBundleEl
                                      );

        virtual PeerFilesPtr generate(
                                      IRSAPrivateKeyPtr privateKey,
                                      IRSAPublicKeyPtr publicKey,
                                      const char *password,
                                      ElementPtr signedSaltBundleEl
                                      );

        virtual PeerFilesPtr loadFromElement(
                                             const char *password,
                                             ElementPtr privatePeerRootElement
                                             );
      };
      
    }
  }
}
