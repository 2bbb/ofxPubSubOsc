//
//  ofxpubsubosc_settings.h
//
//  Created by ISHII 2bit on 2015/09/17.
//
//

#ifndef ofxPubSubDevelopProject_ofxpubsubosc_settings_h
#define ofxPubSubDevelopProject_ofxpubsubosc_settings_h

#pragma mark versioning tags

#define OFX_PUBSUBOSC_VERSION_MAJOR 0
#define OFX_PUBSUBOSC_VERSION_MINOR 2
#define OFX_PUBSUBOSC_VERSION_PATCH 0

#define OFX_PUBSUBOSC_MULTISUBSCRIBE 1

#if (OF_VERSION_MAJOR == 0) && (OF_VERSION_MINOR < 9)
#   error this version uses C++11. (i.e. only oF0.9.0~). you can use old version ( from https://github.com/2bbb/ofxPubSubOsc/releases/tag/v0_1_2 )
#endif

#endif
