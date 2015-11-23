//
//  ofxpubsubosc_settings.h
//
//  Created by ISHII 2bit on 2015/09/17.
//
//

#ifndef ofxPubSubDevelopProject_ofxpubsubosc_settings_h
#define ofxPubSubDevelopProject_ofxpubsubosc_settings_h

#if (OF_VERSION_MAJOR == 0) && ((OF_VERSION_MINOR < 8) || ((OF_VERSION_MINOR == 8) && (OF_VERSION_PATCH < 2)))
#   define ENABLE_OF_BUFFER 0
#else
#   define ENABLE_OF_BUFFER 1
#endif

#if (OF_VERSION_MAJOR == 0) && OF_VERSION_MINOR < 9
#   define ENABLE_FUNCTIONAL 0
#else
#   define ENABLE_FUNCTIONAL 1
#endif

#if __cplusplus <= 199711L
#   define ENABLE_CPP11 0
#else
#   define ENABLE_CPP11 1
#endif

#endif
