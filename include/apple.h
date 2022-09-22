#ifndef APPLE_H
#define APPLE_H

bool apple_delete_file(std::string filename);
#if defined IOS || defined MAS
bool apple_is_logged_into_icloud();
#endif

#endif
