#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();
	al_init_primitives_addon();

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);

	ALLEGRO_BITMAP *img = al_load_bitmap(argv[1]);

	int w = al_get_bitmap_width(img);
	int h = al_get_bitmap_height(img);

	ALLEGRO_BITMAP *dest = al_create_bitmap(w, h);

	al_set_target_bitmap(dest);

	al_clear_to_color(al_map_rgba(0, 0, 0, 0));

	ALLEGRO_VERTEX v[6];

	float tilt_x = 0.25f;
	float tilt_y = 0.65f;

	v[0].x = w * tilt_x;
	v[0].y = h * tilt_y;
	v[0].z = 0;
	v[0].color = al_map_rgba(0, 0, 0, 80);
	v[0].u = 0;
	v[0].v = 0;

	v[1].x = w + w * tilt_x;
	v[1].y = h * tilt_y;
	v[1].z = 0;
	v[1].color = al_map_rgba(0, 0, 0, 80);
	v[1].u = w - 1;
	v[1].v = 0;

	v[2].x = 0;
	v[2].y = h - 1 - (h * 0.05f);
	v[2].z = 0;
	v[2].color = al_map_rgba(0, 0, 0, 80);
	v[2].u = 0;
	v[2].v = h - 1;

	v[3].x = w + w * tilt_x;
	v[3].y = h * tilt_y;
	v[3].z = 0;
	v[3].color = al_map_rgba(0, 0, 0, 80);
	v[3].u = w - 1;
	v[3].v = 0;

	v[4].x = w - 1;
	v[4].y = h - 1 - (h * 0.05f);
	v[4].z = 0;
	v[4].color = al_map_rgba(0, 0, 0, 80);
	v[4].u = w - 1;
	v[4].v = h - 1;
	
	v[5].x = 0;
	v[5].y = h - 1 - (h * 0.05f);
	v[5].z = 0;
	v[5].color = al_map_rgba(0, 0, 0, 80);
	v[5].u = 0;
	v[5].v = h - 1;

	al_draw_prim(v, 0, img, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);

	al_save_bitmap("out.png", dest);
}
