#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>
#include<conio.h>

extern "C" {
#include"SDL2-2.0.10/include/SDL.h"
#include"SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define OBJECTS_IN_ROW  7


//--------Row properties--------
struct row {
        double start = 0.0;                                             
        int freq = 30;                                                  
        bool orientation = rand() % 2;                  // right (true)- left(false)
        int single_shift = 1;                                   
        int shift = 0;
};//

//--------Updating row--------
void update_row(row& r, double counter) {
        if (counter - r.start > r.freq) {                //if there was n*seconds 
                if (r.orientation) {                             
                        
                        r.shift += r.single_shift;               
                        r.shift = r.shift % (2* SCREEN_WIDTH);  
                }
                else {
                        r.shift -= r.single_shift;
                        if (r.shift < 0)        r.shift +=(2* SCREEN_WIDTH);
                }
                r.start = SDL_GetTicks();                       
        }
}//


//--------Collision detection--------
bool Collision(SDL_Surface* sprite_1, SDL_Surface* sprite_2,int x_1, int y_1, int x_2, int y_2) {
        bool kolizja = false;

        if ( ((x_1 + sprite_1->w / 2) < (x_2 - sprite_2->w / 2) && y_1==y_2)    ||              // same 'y', different 'x' (on the right)
                ((x_1 - sprite_1->w / 2) > (x_2 + sprite_2->w / 2) && y_1 == y_2)   ||          // same 'y', different 'x' (on the left)
                (y_1 != y_2 ) ){
                        kolizja = false;
        }//if
        else kolizja = true;

        return kolizja;
}


//--------Objects properties--------
// object - parameters for the object
// count - number of objects in one row
// shift_1 - position 
// frequ - frequency of updating the row
void Set_Object_Param(row object[], int count, int shift_1, int frequ, bool direction) {
        int ii;

        for (ii = 0; ii < count; ii++) {
                object[ii].orientation = direction; 
                object[ii].shift = ii * shift_1 ;        
                object[ii].freq = frequ;        
        }
}


// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
        int px, py, c;
        SDL_Rect s, d;
        s.w = 8;
        s.h = 8;
        d.w = 8;
        d.h = 8;
        while(*text) {
                c = *text & 255;
                px = (c % 16) * 8;
                py = (c / 16) * 8;
                s.x = px;
                s.y = py;
                d.x = x;
                d.y = y;
                SDL_BlitSurface(charset, &s, screen, &d);
                x += 8;
                text++;
                };
        };


// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
        SDL_Rect dest;
        dest.x = x - sprite->w / 2;
        dest.y = y - sprite->h / 2;
        dest.w = sprite->w;
        dest.h = sprite->h;
        SDL_BlitSurface(sprite, NULL, screen, &dest);
        };

// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
        int bpp = surface->format->BytesPerPixel;
        Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
        *(Uint32 *)p = color;
        };

// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
        for(int i = 0; i < l; i++) {
                DrawPixel(screen, x, y, color);
                x += dx;
                y += dy;
                };
        };

// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
        int i;
        DrawLine(screen, x, y, k, 0, 1, outlineColor);
        DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
        DrawLine(screen, x, y, l, 1, 0, outlineColor);
        DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
        for(i = y + 1; i < y + k - 1; i++)
                DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
        };


//--------Main--------
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
        srand((unsigned int)time(NULL));
        Uint32 t1, t2;
        int frames;
        double delta, worldTime, fpsTimer, fps, distance, rc;
        SDL_Event event;
        SDL_Surface *screen, *charset;
        SDL_Surface *zabka, *auto1, *auto2, *drewno, *plansza, *zolwie, *longwood, *leaf_clear, *leaf_0, *leaf_1, *leaf_2, *leaf_3, *leaf_4, *zabka_meta, *game_over, *paused, *quit_game;
        SDL_Texture *scrtex;
        SDL_Window *window;
        SDL_Renderer *renderer;

        int zabkax = SCREEN_WIDTH / 2;
        int zabkay = SCREEN_HEIGHT - 50;
        double counter = 0.0;                           
        row objects[9][OBJECTS_IN_ROW];         
        bool kolizja = false;
        bool meta[5];
        bool zmiana = false;
        bool zycia_minus = false;
        bool pause = false;
        bool quit = false;
        bool quit_temp = false;
        bool g_over = false;
        int life = 3;
        double czas_start, czas_limit = 40.0;
        double t_czarny, t_niebieski = 0.0;
        SDL_Surface* autko = NULL;
        
        for (int ii = 0; ii < 5; ii++) {
                meta[ii] = true;
        }

        //--------Properties of every row--------
        Set_Object_Param(objects[1], OBJECTS_IN_ROW, 120 + rand() % 90, 30, true);              
        Set_Object_Param(objects[2], OBJECTS_IN_ROW, 150 + rand() % 50, 30, false);             
        Set_Object_Param(objects[3], OBJECTS_IN_ROW, 150 + rand() % 50, 70, true);              
        Set_Object_Param(objects[5], OBJECTS_IN_ROW, 150, 50, false);                                   
        Set_Object_Param(objects[6], OBJECTS_IN_ROW, 200, 30, true);                                    
        Set_Object_Param(objects[7], OBJECTS_IN_ROW, 200, 70, false);                                   
        objects[0][0].shift = 55;               
        objects[0][1].shift = 190;              
        objects[0][2].shift = 325;              
        objects[0][3].shift = 460;              
        objects[0][4].shift = 595;              


        //--------SDL Graphics--------
        if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
                printf("SDL_Init error: %s\n", SDL_GetError());
                return 1;
                }
        // fullscreen mode
		// rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,&window, &renderer);
		
        rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
        if(rc != 0) {
                SDL_Quit();
                printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
                return 1;
                };
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_SetWindowTitle(window, "Frogger");

        screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                      0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   SCREEN_WIDTH, SCREEN_HEIGHT);


        SDL_ShowCursor(SDL_DISABLE);

        char text[128];
        int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
        int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
        int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
        int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

        //--------Time settings--------
        t1 = SDL_GetTicks();
        czas_start = 0;
        frames = 0;
        fpsTimer = 0;
        fps = 0;
        worldTime = 0;
        distance = 0;


        //--------Uploading the objects--------
        {
                charset = SDL_LoadBMP_RW(SDL_RWFromFile("objects/cs8x8.bmp", "rb"), 1);
                if (charset == NULL) {
                        printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                SDL_SetColorKey(charset, true, 0x000000);

                zabka = SDL_LoadBMP_RW(SDL_RWFromFile("objects/zabka.bmp", "rb"), 1);
                if (zabka == NULL) {
                        printf("SDL_LoadBMP(zabka.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };

                zabka_meta = SDL_LoadBMP_RW(SDL_RWFromFile("objects/zabka-meta.bmp", "rb"), 1);
                if (zabka_meta == NULL) {
                        printf("SDL_LoadBMP(zabka-meta.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                auto1 = SDL_LoadBMP_RW(SDL_RWFromFile("objects/auto1.bmp", "rb"), 1);
                if (auto1 == NULL) {
                        printf("SDL_LoadBMP(auto1.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };

                auto2 = SDL_LoadBMP_RW(SDL_RWFromFile("objects/auto2.bmp", "rb"), 1);
                if (auto2 == NULL) {
                        printf("SDL_LoadBMP(auto2.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };

                drewno = SDL_LoadBMP_RW(SDL_RWFromFile("objects/drewno.bmp", "rb"), 1);
                if (drewno == NULL) {
                        printf("SDL_LoadBMP(drewno.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };

                plansza = SDL_LoadBMP_RW(SDL_RWFromFile("objects/plansza_1.bmp", "rb"), 1);
                if (plansza == NULL) {
                        printf("SDL_LoadBMP(plansza_1.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };

                zolwie = SDL_LoadBMP_RW(SDL_RWFromFile("objects/zolwie.bmp", "rb"), 1);
                if (zolwie == NULL) {
                        printf("SDL_LoadBMP(zolwie.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };

                longwood = SDL_LoadBMP_RW(SDL_RWFromFile("objects/longwood.bmp", "rb"), 1);
                if (longwood == NULL) {
                        printf("SDL_LoadBMP(longwood.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                leaf_clear = SDL_LoadBMP_RW(SDL_RWFromFile("objects/leaf_clear.bmp", "rb"), 1);
                if (leaf_clear == NULL) {
                        printf("SDL_LoadBMP(leaf_clear.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                leaf_0 = SDL_LoadBMP_RW(SDL_RWFromFile("objects/leaf.bmp", "rb"), 1);
                if (leaf_0 == NULL) {
                        printf("SDL_LoadBMP(leaf.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                leaf_1 = SDL_LoadBMP_RW(SDL_RWFromFile("objects/leaf.bmp", "rb"), 1);
                if (leaf_1 == NULL) {
                        printf("SDL_LoadBMP(leaf.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                leaf_2 = SDL_LoadBMP_RW(SDL_RWFromFile("objects/leaf.bmp", "rb"), 1);
                if (leaf_2 == NULL) {
                        printf("SDL_LoadBMP(leaf.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                leaf_3 = SDL_LoadBMP_RW(SDL_RWFromFile("objects/leaf.bmp", "rb"), 1);
                if (leaf_3 == NULL) {
                        printf("SDL_LoadBMP(leaf.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                leaf_4 = SDL_LoadBMP_RW(SDL_RWFromFile("objects/leaf.bmp", "rb"), 1);
                if (leaf_4 == NULL) {
                        printf("SDL_LoadBMP(leaf.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };

                game_over = SDL_LoadBMP_RW(SDL_RWFromFile("objects/game_over.bmp", "rb"), 1);
                if (game_over == NULL) {
                        printf("SDL_LoadBMP(game_over.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                paused = SDL_LoadBMP_RW(SDL_RWFromFile("objects/paused.bmp", "rb"), 1);
                if (paused == NULL) {
                        printf("SDL_LoadBMP(paused.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
                quit_game = SDL_LoadBMP_RW(SDL_RWFromFile("objects/quit_game.bmp", "rb"), 1);
                if (quit_game == NULL) {
                        printf("SDL_LoadBMP(quit_game.bmp) error: %s\n", SDL_GetError());
                        SDL_FreeSurface(charset);
                        SDL_FreeSurface(screen);
                        SDL_DestroyTexture(scrtex);
                        SDL_DestroyWindow(window);
                        SDL_DestroyRenderer(renderer);
                        SDL_Quit();
                        return 1;
                };
        }


                //--------Game--------
                while (quit == false) {
                        if (pause == false && g_over == false) {

                                t2 = SDL_GetTicks();
                                delta = (t2 - t1) * 0.001;
                                t1 = t2;

                                counter = SDL_GetTicks();

                                worldTime += delta;

                                SDL_FillRect(screen, NULL, czarny);
                                DrawSurface(screen, plansza, 322, 255);


                                //--------Objects movement--------
                                //drewno
                                kolizja = false;
                                for (int ii = 0; ii < OBJECTS_IN_ROW; ii++) {            
                                        update_row(objects[1][ii], counter);
                                        DrawSurface(screen, drewno, 15 + objects[1][ii].shift, 115);
                                        if (Collision(drewno, zabka, objects[1][ii].shift, 115, zabkax, zabkay)) {
                                                if (zabkax < SCREEN_WIDTH) {                                     
                                                        update_row(objects[8][0], counter);
                                                        DrawSurface(screen, zabka, zabkax, 115);
                                                }
                                                else {
                                                        zabkax = SCREEN_WIDTH / 2;
                                                        zabkay = SCREEN_HEIGHT - 50;
                                                        update_row(objects[8][0], counter);
                                                        DrawSurface(screen, zabka, zabkax, zabkay);
                                                        life--;
                                                }
                                                kolizja = true;
                                        }
                                }
                                if (zabkay == 115 && kolizja == false) {
                                        zabkax = SCREEN_WIDTH / 2;
                                        zabkay = SCREEN_HEIGHT - 50;
                                        update_row(objects[8][0], counter);
                                        DrawSurface(screen, zabka, zabkax, zabkay);
                                        life--;

                                }
                                
                                //dlugie drewno
                                kolizja = false;
                                for (int ii = 0; ii < OBJECTS_IN_ROW; ii++) {            
                                        update_row(objects[2][ii], counter);
                                        DrawSurface(screen, longwood, 40 + objects[2][ii].shift, 160);
                                        if (Collision(longwood, zabka, 40 + objects[2][ii].shift, 160, zabkax, zabkay)) {
                                                if (zabkax < SCREEN_WIDTH) {                                     
                                                        update_row(objects[8][0], counter);
                                                        DrawSurface(screen, zabka, zabkax, 160);
                                                }
                                                else {
                                                        zabkax = SCREEN_WIDTH / 2;
                                                        zabkay = SCREEN_HEIGHT - 50;
                                                        update_row(objects[8][0], counter);
                                                        DrawSurface(screen, zabka, zabkax, zabkay);
                                                        life--;
                                                }
                                                kolizja = true;
                                        }
                                }
                                if (zabkay == 160 && kolizja == false) {
                                        zabkax = SCREEN_WIDTH / 2;
                                        zabkay = SCREEN_HEIGHT - 50;
                                        update_row(objects[8][0], counter);
                                        DrawSurface(screen, zabka, zabkax, zabkay);
                                        life--;

                                }

                                //zolwie
                                kolizja = false;
                                for (int ii = 0; ii < OBJECTS_IN_ROW; ii++) {
                                        update_row(objects[3][ii], counter);
                                        DrawSurface(screen, zolwie, 40 + objects[3][ii].shift, 205);

                                        if (Collision(zolwie, zabka, 40 + objects[3][ii].shift, 205, zabkax, zabkay)) {
                                                if (zabkax < SCREEN_WIDTH) {                                     
                                                        update_row(objects[8][0], counter);
                                                        DrawSurface(screen, zabka, zabkax, 205);
                                                }
                                                else {
                                                        zabkax = SCREEN_WIDTH / 2;
                                                        zabkay = SCREEN_HEIGHT - 50;
                                                        update_row(objects[8][0], counter);
                                                        DrawSurface(screen, zabka, zabkax, zabkay);
                                                }
                                                kolizja = true;
                                        }
                                }
                                if (zabkay == 205 && kolizja == false) {
                                        zabkax = SCREEN_WIDTH / 2;
                                        zabkay = SCREEN_HEIGHT - 50;
                                        update_row(objects[8][0], counter);
                                        DrawSurface(screen, zabka, zabkax, zabkay);
                                        life--;
                                }
                                
                                //auto1 i auto2 i auto3
                                for (int jj = 5; jj < 8; jj++) {
                                        for (int ii = 0; ii < OBJECTS_IN_ROW; ii++) {
                                                update_row(objects[jj][ii], counter);
                                                if (jj < 7) { autko = auto1; }
                                                else { autko = auto2; }
                                                DrawSurface(screen, autko, 40 + objects[jj][ii].shift, (295 + (jj%5 * 45)) );
                                                if (Collision(auto1, zabka, 40 + objects[jj][ii].shift, (295 + (jj % 5 * 45)), zabkax, zabkay)) {
                                                        zabkax = SCREEN_WIDTH / 2;
                                                        zabkay = SCREEN_HEIGHT - 50;
                                                        update_row(objects[8][0], counter);
                                                        DrawSurface(screen, zabka, zabkax, zabkay);
                                                        life--;
                                                }
                                        }
                                }
                

                                //zabka
                                for (int ii = 0; ii < OBJECTS_IN_ROW; ii++) {
                                        DrawSurface(screen, zabka, zabkax, zabkay);
                                        if (zabkax < 0) {
                                                zabkax = 0 + zabka->w;
                                                DrawSurface(screen, zabka, zabkax, zabkay);
                                        }
                                
                                        if (Collision(auto1, zabka, 40 + objects[5][ii].shift, 295, zabkax, zabkay)) {
                                                zabkax = SCREEN_WIDTH / 2;
                                                zabkay = SCREEN_HEIGHT - 50;
                                                DrawSurface(screen, zabka, zabkax, zabkay);
                                                life--;
                                        }
                                        if (Collision(auto2, zabka, 40 + objects[6][ii].shift, 340, zabkax, zabkay)) {
                                                zabkax = SCREEN_WIDTH / 2;
                                                zabkay = SCREEN_HEIGHT - 50;
                                                DrawSurface(screen, zabka, zabkax, zabkay);
                                                life--;
                                        }
                                        if (Collision(auto2, zabka, 40 + objects[7][ii].shift, 385, zabkax, zabkay)) {
                                                zabkax = SCREEN_WIDTH / 2;
                                                zabkay = SCREEN_HEIGHT - 50;
                                                update_row(objects[8][0], counter);
                                                DrawSurface(screen, zabka, zabkax, zabkay);
                                                life--;
                                        }
                                        if (Collision(drewno, zabka, 13 + objects[1][ii].shift, 115, zabkax, zabkay)) {
                                                zabkax = 13 + objects[1][ii].shift;
                                                update_row(objects[8][0], counter);
                                                DrawSurface(drewno, zabka, zabkax, zabkay);

                                        }
                                        if (Collision(longwood, zabka, 40 + objects[2][ii].shift, 160, zabkax, zabkay)) {
                                                zabkax = 40 + objects[2][ii].shift;
                                                update_row(objects[8][0], counter);
                                                DrawSurface(longwood, zabka, zabkax, zabkay);
                                        }
                                        if (Collision(zolwie, zabka, 40 + objects[3][ii].shift, 205, zabkax, zabkay)) {
                                                zabkax = 40 + objects[3][ii].shift;
                                                update_row(objects[8][0], counter);
                                                DrawSurface(zolwie, zabka, zabkax, zabkay);
                                        }

                                }


                        //--------Finish line--------
                                DrawSurface(screen, leaf_0, 55, 70);            
                                DrawSurface(screen, leaf_1, 190, 70);           
                                DrawSurface(screen, leaf_2, 325, 70);           
                                DrawSurface(screen, leaf_3, 460, 70);           
                                DrawSurface(screen, leaf_4, 595, 70);           

                                fpsTimer += delta;
                                if (fpsTimer > 0.5) {
                                        fps = frames * 2.0;
                                        frames = 0;
                                        fpsTimer -= 0.5;
                                };

                        }

                        //--------Handling events--------
                        while (SDL_PollEvent(&event)) {
                                switch (event.type) {
                                case SDL_KEYDOWN:
                                        if (event.key.keysym.sym == SDLK_ESCAPE) quit = true;
                                        else if (!pause && (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)){
                                                zabkay -= 45;
                                                if (zabkay == 70) {
                                                        zmiana = false;

                                                        kolizja = Collision(leaf_0, zabka, objects[0][0].shift, 70, zabkax, zabkay);
                                                        if (meta[0] == true && kolizja == true ) {                                       
                                                                SDL_BlitSurface(zabka_meta, NULL, leaf_0, NULL);                
                                                                meta[0] = false;
                                                                zmiana = true;
                                                        }
                                                        kolizja = Collision(leaf_1, zabka, objects[0][1].shift, 70, zabkax, zabkay);
                                                        if (meta[1] == true && kolizja == true) {
                                                                SDL_BlitSurface(zabka_meta, NULL, leaf_1, NULL);                
                                                                meta[1] = false;
                                                                zmiana = true;
                                                        }
                                                        kolizja = Collision(leaf_2, zabka, objects[0][2].shift, 70, zabkax, zabkay);
                                                        if (meta[2] == true && kolizja == true) {
                                                                SDL_BlitSurface(zabka_meta, NULL, leaf_2, NULL);               
                                                                meta[2] = false;
                                                                zmiana = true;
                                                        }
                                                        kolizja = Collision(leaf_3, zabka, objects[0][3].shift, 70, zabkax, zabkay);
                                                        if (meta[3] == true && kolizja == true) {
                                                                SDL_BlitSurface(zabka_meta, NULL, leaf_3, NULL);                
                                                                meta[3] = false;
                                                                zmiana = true;
                                                        }
                                                        kolizja = Collision(leaf_4, zabka, objects[0][4].shift, 70, zabkax, zabkay);
                                                        if (meta[4] == true && kolizja == true) {
                                                                SDL_BlitSurface(zabka_meta, NULL, leaf_4, NULL);              
                                                                meta[4] = false;
                                                                zmiana = true;
                                                        }
                                                        
                                                        if (zmiana == false) {                  
                                                                life--;
                                                        }
                                                        
                                                        zabkax = SCREEN_WIDTH / 2;
                                                        zabkay = SCREEN_HEIGHT - 50;
                                                        DrawSurface(screen, zabka, zabkax, zabkay);

                                                }//if zabkay==70

                                        }//else if                                              

                                        else if (!pause && (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)) {
                                        if (zabkay < SCREEN_HEIGHT - 50)
                                                zabkay += 45;
                                        }

                                        else if (!pause && (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)) {
                                        zabkax += 45;
                                        if (zabkax > SCREEN_WIDTH) {
                                                zabkax = SCREEN_WIDTH / 2;
                                                zabkay = SCREEN_HEIGHT - 50;

                                                DrawSurface(screen, zabka, zabkax, zabkay);
                                        }
                                        }
                                        else if (!pause && (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)) {
                                        zabkax -= 45;
                                        if (zabkax < 0) {
                                                zabkax = SCREEN_WIDTH / 2;
                                                zabkay = SCREEN_HEIGHT - 50;
                                                DrawSurface(screen, zabka, zabkax, zabkay);
                                        }
                                        }
                                        //--------Pause
                                        else if (event.key.keysym.sym == SDLK_p) {
                                        pause = !pause;
                                        if (!pause) {
                                                t1 = SDL_GetTicks();    
                                        }
                                        }
                                        //--------Quit_game
                                        else if (!pause && event.key.keysym.sym == SDLK_q) {
                                        quit_temp = true;
                                        pause = true;
                                        }
                                        else if (quit_temp && event.key.keysym.sym == SDLK_y) {  
                                        quit = true;
                                        }
                                        else if (quit_temp && event.key.keysym.sym == SDLK_n) {  
                                        quit_temp = false;
                                        pause = false;
                                        if (!quit_temp) t1 = SDL_GetTicks();
                                        }
                                        //--------Game over
                                        else if (g_over && event.key.keysym.sym == SDLK_y) {
                                        quit = true;
                                        }
                                        else if (g_over && event.key.keysym.sym == SDLK_n) {             
                                        worldTime = 0.0;
                                        t1 = SDL_GetTicks();
                                        t2 = SDL_GetTicks();

                                        life = 3;
                                        g_over = false;
                                        SDL_BlitSurface(leaf_clear, NULL, leaf_0, NULL);
                                        SDL_BlitSurface(leaf_clear, NULL, leaf_1, NULL);
                                        SDL_BlitSurface(leaf_clear, NULL, leaf_2, NULL);
                                        SDL_BlitSurface(leaf_clear, NULL, leaf_3, NULL);
                                        SDL_BlitSurface(leaf_clear, NULL, leaf_4, NULL);
                                        for (int ii = 0; ii < 5; ii++) {
                                                meta[ii] = true;
                                        }
                                        }
                                        break;
                                case SDL_KEYUP:
                                        break;
                                case SDL_QUIT:
                                        quit = true;
                                        break;
                                };//switch
                        };//while(event)

                        if (life == 0) {
                                DrawSurface(screen, game_over, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
                                g_over = true;
                        }
                        if (pause == true) {
                                DrawSurface(screen, paused, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
                        }
                        if (quit_temp == true) {
                                DrawSurface(screen, quit_game, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
                        }


                        //------Frame with time----
                        t_niebieski = worldTime;
                        t_czarny = czas_limit - t_niebieski;
                        if (t_czarny < 0) {
                                t_czarny = 0;
                        }
                        if (t_niebieski > czas_limit) {
                                t_niebieski = czas_limit;
                        }

                        
                        if  (   (worldTime > czas_limit + 10) && (worldTime < czas_limit + 20)  &&      zycia_minus == false    ){
                        life--;
                        zycia_minus = true;
                        }
                        if ((worldTime > czas_limit + 20) && (worldTime < czas_limit + 30) && zycia_minus == true) {
                                life--;
                                zycia_minus = false;
                        }
                        if ((worldTime > czas_limit + 30) && (worldTime < czas_limit + 40) && zycia_minus == false) {
                                life--;
                                zycia_minus = true;
                        }
                        
                        DrawRectangle(screen, SCREEN_WIDTH + 400, SCREEN_HEIGHT - 29, (int)t_czarny * 5, 25, czarny, czarny);
                        if ((t_niebieski < czas_limit ) ) {
                                DrawRectangle(screen, SCREEN_WIDTH + 400 + (int)t_czarny * 5, SCREEN_HEIGHT - 29, (int)t_niebieski * 5, 25, niebieski, niebieski);
                        }
                        else DrawRectangle(screen, SCREEN_WIDTH + 400 + (int)t_czarny * 5, SCREEN_HEIGHT - 29, (int)t_niebieski * 5, 25, czerwony, czerwony);
                        
                        sprintf_s(text, "czas trwania = %.1lf", worldTime);
                        DrawString(screen, SCREEN_WIDTH / 2 +100 , SCREEN_HEIGHT - 20, text, charset);
                        //-------------

                        {//info text
                                DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
                                sprintf_s(text, "czas trwania = %.1lf zycia = %d ", worldTime, life);
                                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
                                sprintf_s(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie");
                                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

                                SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
                                SDL_RenderCopy(renderer, scrtex, NULL, NULL);
                                SDL_RenderPresent(renderer);
                        }

                }; //while(quit)

        //freeing all surfaces
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();
        return 0;
        };//main