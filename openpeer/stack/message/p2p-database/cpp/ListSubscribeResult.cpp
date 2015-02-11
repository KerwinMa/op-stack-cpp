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

#include <openpeer/stack/message/internal/stack_message_MessageHelper.h>
#include <openpeer/stack/message/p2p-database/ListSubscribeRequest.h>
#include <openpeer/stack/message/p2p-database/ListSubscribeResult.h>

#include <openpeer/stack/IPublication.h>

namespace openpeer
{
  namespace stack
  {
    namespace message
    {
      namespace p2p_database
      {
        //---------------------------------------------------------------------
        ListSubscribeResultPtr ListSubscribeResult::convert(MessagePtr message)
        {
          return ZS_DYNAMIC_PTR_CAST(ListSubscribeResult, message);
        }

        //---------------------------------------------------------------------
        ListSubscribeResult::ListSubscribeResult()
        {
          mAppID.clear();
        }

        //---------------------------------------------------------------------
        ListSubscribeResultPtr ListSubscribeResult::create(ListSubscribeRequestPtr request)
        {
          ListSubscribeResultPtr ret(new ListSubscribeResult);
          ret->fillFrom(request);
          return ret;
        }

        //---------------------------------------------------------------------
        ListSubscribeResultPtr ListSubscribeResult::create(
                                                           ElementPtr root,
                                                           IMessageSourcePtr messageSource
                                                           )
        {
          ListSubscribeResultPtr ret(new ListSubscribeResult);
          IMessageHelper::fill(*ret, root, messageSource);

          return ret;
        }

        //---------------------------------------------------------------------
        DocumentPtr ListSubscribeResult::encode()
        {
          DocumentPtr ret = IMessageHelper::createDocumentWithRoot(*this);
          return ret;
        }

        //---------------------------------------------------------------------
        bool ListSubscribeResult::hasAttribute(ListSubscribeResult::AttributeTypes type) const
        {
          switch (type)
          {
          }
          return MessageResult::hasAttribute((MessageResult::AttributeTypes)type);
        }

      }
    }
  }
}