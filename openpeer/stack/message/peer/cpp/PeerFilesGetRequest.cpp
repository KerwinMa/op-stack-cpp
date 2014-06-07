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

#include <openpeer/stack/message/peer/PeerFilesGetRequest.h>
#include <openpeer/stack/message/internal/stack_message_MessageHelper.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/XML.h>
#include <zsLib/helpers.h>

#define OPENPEER_STACK_MESSAGE_PEER_FILES_GET_REQUEST_EXPIRES_TIME_IN_SECONDS ((60*60)*24)

namespace openpeer { namespace stack { namespace message { ZS_DECLARE_SUBSYSTEM(openpeer_stack_message) } } }

namespace openpeer
{
  namespace stack
  {
    namespace message
    {
      using services::IHelper;

      namespace peer
      {
        using zsLib::Seconds;
        using internal::MessageHelper;

        //---------------------------------------------------------------------
        PeerFilesGetRequestPtr PeerFilesGetRequest::convert(MessagePtr message)
        {
          return dynamic_pointer_cast<PeerFilesGetRequest>(message);
        }

        //---------------------------------------------------------------------
        PeerFilesGetRequest::PeerFilesGetRequest()
        {
        }

        //---------------------------------------------------------------------
        PeerFilesGetRequestPtr PeerFilesGetRequest::create()
        {
          PeerFilesGetRequestPtr ret(new PeerFilesGetRequest);
          return ret;
        }

        //---------------------------------------------------------------------
        bool PeerFilesGetRequest::hasAttribute(AttributeTypes type) const
        {
          switch (type)
          {
            case AttributeType_PeerURIs:      return mPeerURIs.size() > 0;
            default:                          break;
          }
          return false;
        }

        //---------------------------------------------------------------------
        DocumentPtr PeerFilesGetRequest::encode()
        {
          DocumentPtr ret = IMessageHelper::createDocumentWithRoot(*this);
          ElementPtr rootEl = ret->getFirstChildElement();

          String clientNonce = IHelper::randomString(32);
          Time expires = zsLib::now() + Seconds(OPENPEER_STACK_MESSAGE_PEER_FILES_GET_REQUEST_EXPIRES_TIME_IN_SECONDS);

          ElementPtr peersEl = Element::create("peers");

          if (hasAttribute(AttributeType_PeerURIs)) {

            for (PeerURIMap::const_iterator iter = mPeerURIs.begin(); iter != mPeerURIs.end(); ++iter)
            {
              ElementPtr peerEl = Element::create("peer");
              const PeerURI &uri = (*iter).first;
              const PeerFindSecret &findSecret = (*iter).second;

              if (uri.hasData()) {
                peerEl->adoptAsLastChild(IMessageHelper::createElementWithTextAndJSONEncode("uri", uri));
              }

              if (findSecret.hasData()) {
                String findSecretProof = IHelper::convertToHex(*IHelper::hmac(*IHelper::hmacKeyFromPassphrase(findSecret), "proof:" + clientNonce + ":" + IHelper::timeToString(expires)));
                peerEl->adoptAsLastChild(IMessageHelper::createElementWithTextAndJSONEncode("findSecretNonce", clientNonce));
                peerEl->adoptAsLastChild(IMessageHelper::createElementWithTextAndJSONEncode("findSecretProof", findSecretProof));
                peerEl->adoptAsLastChild(IMessageHelper::createElementWithNumber("findSecretProofExpires", IHelper::timeToString(expires)));
              }

              if (peerEl->hasChildren()) {
                peersEl->adoptAsLastChild(peerEl);
              }

            }
          }

          if (peersEl->hasChildren()) {
            rootEl->adoptAsLastChild(peersEl);
          }
          
          return ret;
        }
      }
    }
  }
}
