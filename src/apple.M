#include <string>

#import <Foundation/Foundation.h>

#if defined IOS || defined MAS
#import <CloudKit/CloudKit.h>
#endif

#include "Nooskewl_Shim/main.h"

#include "apple.h"

bool apple_delete_file(std::string filename)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	BOOL ret = [[NSFileManager defaultManager] removeItemAtPath:[NSString stringWithUTF8String:filename.c_str()] error:nil];
	[pool release];
	return ret;
}

#if defined IOS || defined MAS
static void apple_create_icloud_schema()
{
	CKRecordID *m3RecordID = [[CKRecordID alloc] initWithRecordName:@"MoRPG3"];
	CKRecord *m3Record = [[CKRecord alloc] initWithRecordType:@"m3Saves" recordID:m3RecordID];
	char bytes[1] = { 0 };
	m3Record[@"auto1_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto2_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto3_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto4_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto5_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save1_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save2_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save3_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save4_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save5_txt"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto1_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto2_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto3_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto4_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"auto5_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save1_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save2_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save3_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save4_dat"] = [NSData dataWithBytes:bytes length:0];
	m3Record[@"save5_dat"] = [NSData dataWithBytes:bytes length:0];
	CKContainer *myContainer = [CKContainer containerWithIdentifier:@"iCloud.ca.nooskewl.monsterrpg3"];
	CKDatabase *publicDatabase = [myContainer publicCloudDatabase];
	[publicDatabase saveRecord:m3Record completionHandler:^(CKRecord *m3Record, NSError *error){
		if (error) {
			printf("Error!\n");
			return;
		}
		printf("OK!\n");
	}];
}

bool apple_is_logged_into_icloud()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	CKContainer *myContainer = [CKContainer containerWithIdentifier:@"iCloud.ca.nooskewl.monsterrpg3"];
	__block int done = -1;
	[myContainer accountStatusWithCompletionHandler:^(CKAccountStatus accountStatus, NSError *error) {
		if (error) {
			done = 0;
		}
		else {
			apple_create_icloud_schema();
			done = 1;
		}
	}];
	while (done == -1) {
		SDL_Delay(1);
	}
	[pool release];
	return done;
}
#endif
