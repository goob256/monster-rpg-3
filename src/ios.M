#import <UIKit/UIKit.h>

#include "Nooskewl_Shim/main.h"
#include "Nooskewl_Shim/mml.h"
#include "Nooskewl_Shim/shim.h"
#include "Nooskewl_Shim/util.h"

#include "Nooskewl_Wedge/globals.h"

using namespace noo;	

static bool license_done;
static float old_volume;

static void all_done()
{
	shim::music->play(old_volume, true);
	UIViewController *root = [[[UIApplication sharedApplication] keyWindow] rootViewController];
	[root dismissViewControllerAnimated:YES completion:nil];
	license_done = true;
}

@interface MyTextView : UITextView
{
}
#ifdef TVOS
- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
#endif
@end

@implementation MyTextView
#ifdef TVOS
- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
	[super pressesBegan:presses withEvent:event];
	for (id value in presses) {
		UIPress *p = value;
		if (p.type == UIPressTypeMenu) {
			all_done();
		}
	}
}
- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
	[super pressesEnded:presses withEvent:event];
}
#endif
@end

@interface LicenseViewController : UIViewController
{
	UIViewController *parent;
}
- (id)initWithHTML:(NSAttributedString *)text;
- (void)done;
@end

@implementation LicenseViewController
- (id) initWithHTML:(NSAttributedString *)text
{
	self = [super initWithNibName:nil bundle:nil];

	// Size doesn't seem to matter...
	CGRect f;
	f.origin.x = 0;
	f.origin.y = 0;
	f.size.width = 0;
	f.size.height = 0;
	MyTextView *text_view = [[MyTextView alloc] initWithFrame:f];
	text_view.attributedText = text;
#ifndef TVOS
	text_view.editable = FALSE;
#endif
	text_view.selectable = TRUE;
	text_view.userInteractionEnabled = TRUE;
#ifdef TVOS
	text_view.panGestureRecognizer.allowedTouchTypes = @[ @(UITouchTypeDirect), @(UITouchTypeIndirect) ];
#endif

	self.view = text_view;

#ifndef TVOS
	UIBarButtonItem *bbi = [[UIBarButtonItem alloc] initWithTitle:@"Done" style:UIBarButtonItemStyleDone target:self action:@selector(done)];
	[self.navigationItem setRightBarButtonItem:bbi animated:YES];
#endif

	self.edgesForExtendedLayout = UIRectEdgeNone;

	return self;
}
- (void)done
{
	all_done();
}
@end

bool ios_show_text(std::string fn_s)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	old_volume = shim::music->get_master_volume();

	NSString *fn = [NSString stringWithUTF8String:fn_s.c_str()];

	NSURL *path = [[NSBundle mainBundle] URLForResource:fn withExtension:@"html"];
	NSAttributedString *html = [[NSAttributedString alloc] initWithURL:path options:@{NSDocumentTypeDocumentAttribute:NSHTMLTextDocumentType} documentAttributes:nil error:nil];
	if (html == nil) {
		[pool release];
		return false;
	}

	license_done = false;

	SDL_SetHint(SDL_HINT_APPLE_TV_CONTROLLER_UI_EVENTS, "1");

	LicenseViewController *license_vc = [[LicenseViewController alloc] initWithHTML:html];

	UINavigationController *nav = [[UINavigationController alloc] initWithRootViewController:license_vc];

	UIViewController *root = [[[UIApplication sharedApplication] keyWindow] rootViewController];

	[root presentViewController:nav animated:YES completion:nil];

	while (license_done == false) {
		SDL_PumpEvents();
		SDL_Delay(1);
	}

	SDL_SetHint(SDL_HINT_APPLE_TV_CONTROLLER_UI_EVENTS, "0");
	
	SDL_PumpEvents();
	SDL_FlushEvents(0, 0xffffffff);

	[pool release];

	return true;
}

bool ios_show_license()
{
#ifdef TVOS
	return ios_show_text("3rd_party_tvos");
#else
	return ios_show_text("3rd_party");
#endif
}

bool ios_show_manual()
{
#ifdef TVOS
	return ios_show_text("manual_" + util::lowercase(GLOBALS->language) + "_tvos");
#else
	return ios_show_text("manual_" + util::lowercase(GLOBALS->language));
#endif
}
