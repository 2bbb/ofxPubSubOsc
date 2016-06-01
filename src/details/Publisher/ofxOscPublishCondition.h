//
//  ofxOscPublishCondition.h
//
//  Created by ISHII 2bit on 2016/05/31.
//
//

#pragma once

#ifndef ofxOscPublishCondition_h
#define ofxOscPublishCondition_h

#include <string>
#include <functional>

#include "ofxPubSubOscSettings.h"

namespace ofx {
    namespace PubSubOsc {
        namespace Publish {
            struct BasicCondition {
                BasicCondition() : bPublishNow(true) {};
                
                inline bool getCondition() { return isPublishNow() && inner_condition(); };
                
                inline bool isPublishNow() const { return bPublishNow; };
                inline void setEnablePublish(bool bEnablePublish) { this->bPublishNow = bEnablePublish; };
                
                virtual bool inner_condition() { return true; };
                
                using Ref = std::shared_ptr<BasicCondition>;
            private:
                bool bPublishNow;
            };
            
            struct Condition : BasicCondition {
                Condition(bool &ref) : BasicCondition(), ref(ref) {};
                virtual bool inner_condition() { return ref; };
            private:
                bool &ref;
            };
            
            struct ConditionFunction : BasicCondition {
                ConditionFunction(std::function<bool()> getter) : BasicCondition(), getter(getter) {};
                virtual bool inner_condition() { return getter(); };
            private:
                std::function<bool()> getter;
            };
            
            using ConditionRef = BasicCondition::Ref;
        };
    };
};

#endif /* ofxOscPublishCondition_h */
