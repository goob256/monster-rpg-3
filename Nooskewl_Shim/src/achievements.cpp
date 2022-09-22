#include "Nooskewl_Shim/achievements.h"

#if defined IOS || defined MAS
#include "Nooskewl_Shim/gamecenter.h"
#endif

#ifdef STEAMWORKS
#include "Nooskewl_Shim/steamworks.h"
#endif

using namespace noo;

namespace noo {

namespace util {

#if defined GOOGLE_PLAY || defined AMAZON
#include <jni.h>

bool achieve_android(int id)
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "achieve", "(I)V");

	bool ret;
	if (method_id != 0) {
		env->CallVoidMethod(activity, method_id, id);
		ret = true;
	}
	else {
		ret = false;
	}

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return ret;
}

bool show_achievements_android()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "show_achievements", "()Z");
	
	bool ret;
	if (method_id != 0) {
		ret = env->CallBooleanMethod(activity, method_id);
	}
	else {
		ret = false;
	}

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return ret;
}
#endif

bool achieve(void *id)
{
#if defined GOOGLE_PLAY || defined AMAZON
	// hack trick to convert pointer to int
	return achieve_android((int *)id - (int *)0);
#elif defined IOS || defined MAS
	return achieve_gamecenter((char *)id);
#elif defined STEAMWORKS
	return achieve_steam((char *)id);
#else
	return false; // failed
#endif
}

bool show_achievements()
{
#if defined GOOGLE_PLAY || defined AMAZON
	return show_achievements_android();
#elif defined IOS || defined MAS
	return show_achievements_gamecenter();
#else
	return false; // failed
#endif
}

} // End namespace util

} // End namespace noo
