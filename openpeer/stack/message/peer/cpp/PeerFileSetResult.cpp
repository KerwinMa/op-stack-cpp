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

#include <openpeer/stack/message/peer/PeerFileSetResult.h>
#include <openpeer/stack/message/internal/stack_message_MessageHelper.h>
#include <openpeer/stack/IPeerFilePublic.h>

#include <zsLib/XML.h>
#include <zsLib/helpers.h>

namespace openpeer
{
  namespace stack
  {
    namespace message
    {
      namespace peer
      {
        using internal::MessageHelper;

        //---------------------------------------------------------------------
        PeerFileSetResultPtr PeerFileSetResult::convert(MessagePtr message)
        {
          return ZS_DYNAMIC_PTR_CAST(PeerFileSetResult, message);
        }

        //---------------------------------------------------------------------
        PeerFileSetResult::PeerFileSetResult()
        {
        }

        //---------------------------------------------------------------------
        PeerFileSetResultPtr PeerFileSetResult::create(
                                                       ElementPtr root,
                                                       IMessageSourcePtr messageSource
                                                       )
        {
          PeerFileSetResultPtr ret(new PeerFileSetResult);
          IMessageHelper::fill(*ret, root, messageSource);

          ElementPtr servicesEl = root->findFirstChildElement("services");
          if (!servicesEl) return ret;

          ElementPtr serviceEl = servicesEl->findFirstChildElement("service");
          while (serviceEl) {
            Service service = Service::create(serviceEl);
            if (service.hasData()) {
              ret->mServices[service.mID] = service;

              ServiceTypeMap::iterator found = ret->mServicesByType.find(service.mType);
              if (found == ret->mServicesByType.end()) {
                ret->mServicesByType[service.mType] = ServiceMap();
                found = ret->mServicesByType.find(service.mType);
              }

              ServiceMap &changeMap = (*found).second;
              changeMap[service.mID] = service;
            }

            serviceEl = serviceEl->findNextSiblingElement("service");
          }

          return ret;
        }

        //---------------------------------------------------------------------
        bool PeerFileSetResult::hasAttribute(AttributeTypes type) const
        {
          switch (type)
          {
            case AttributeType_Services:    return (mServices.size() > 0);
            default:                        break;
          }
          return MessageResult::hasAttribute((MessageResult::AttributeTypes)type);
        }

      }
    }
  }
}