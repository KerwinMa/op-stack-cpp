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

#include <openpeer/stack/message/push-mailbox/PeerValidateRequest.h>
#include <openpeer/stack/message/internal/stack_message_MessageHelper.h>

#include <openpeer/stack/internal/stack_Stack.h>
#include <openpeer/stack/IPeerFiles.h>
#include <openpeer/stack/IPeerFilePublic.h>
#include <openpeer/stack/IPeerFilePrivate.h>

#include <openpeer/services/IHelper.h>

//#include <zsLib/Stringize.h>
//#include <zsLib/helpers.h>
#include <zsLib/XML.h>

namespace openpeer { namespace stack { namespace message { ZS_DECLARE_SUBSYSTEM(openpeer_stack_message) } } }

namespace openpeer
{
  namespace stack
  {
    namespace message
    {
      using services::IHelper;

      namespace push_mailbox
      {
        using internal::MessageHelper;

        typedef stack::internal::IStackForInternal UseStack;

        using zsLib::Seconds;

        //---------------------------------------------------------------------
        static Log::Params slog(const char *message)
        {
          return Log::Params(message, "PeerValidateRequest");
        }

        //---------------------------------------------------------------------
        PeerValidateRequestPtr PeerValidateRequest::convert(MessagePtr message)
        {
          return dynamic_pointer_cast<PeerValidateRequest>(message);
        }

        //---------------------------------------------------------------------
        PeerValidateRequest::PeerValidateRequest()
        {
          mAppID.clear();
        }

        //---------------------------------------------------------------------
        PeerValidateRequestPtr PeerValidateRequest::create()
        {
          PeerValidateRequestPtr ret(new PeerValidateRequest);
          return ret;
        }

        //---------------------------------------------------------------------
        bool PeerValidateRequest::hasAttribute(AttributeTypes type) const
        {
          switch (type)
          {
            case AttributeType_PeerChallengeID:   return mPeerChallengeID.hasData();
            case AttributeType_PeerFiles:         return (bool)mPeerFiles;
            default:                              break;
          }
          return false;
        }

        //---------------------------------------------------------------------
        DocumentPtr PeerValidateRequest::encode()
        {
          IPeerFilePublicPtr peerFilePublic;
          IPeerFilePrivatePtr peerFilePrivate;
          if (mPeerFiles) {
            peerFilePublic =  mPeerFiles->getPeerFilePublic();
            peerFilePrivate = mPeerFiles->getPeerFilePrivate();
          }
          
          DocumentPtr ret = IMessageHelper::createDocumentWithRoot(*this);
          ElementPtr rootEl = ret->getFirstChildElement();


          ElementPtr peerProofBundleEl = Element::create("peerProofBundle");
          ElementPtr peerProofEl = (mPeerChallengeID.hasData() ? IMessageHelper::createElementWithID("peerProof", mPeerChallengeID) : Element::create("peerProof"));

          if (peerFilePublic) {
            ElementPtr peerEl = peerFilePublic->saveToElement();
            peerProofEl->adoptAsLastChild(peerEl);
          }

          peerProofBundleEl->adoptAsLastChild(peerProofEl);

          if (peerFilePrivate) {
            peerFilePrivate->signElement(peerProofEl);
          }

          return ret;
        }


      }
    }
  }
}
