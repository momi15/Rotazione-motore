#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
//SDL things
#define ALTEZZA_TITOLO 20
#define ALTEZZA_S_BOTTONI 4
#define Y_BOTTONI 4
#define MARGINE 100
#define RIDUTTORE 0.5
//socket things
#define PORT 27016
#define IP ""

template <typename var>
void controllo(var* variable){
    if(variable==nullptr){
        std::cerr<<"error: "<<SDL_GetError()<<"\n";
        exit(0);
    }
}

struct Windows
{
    SDL_Window *Window{nullptr};
    SDL_Renderer *Renderer{nullptr};
    SDL_Texture *Texture{nullptr};
    size_t w{0},h{0};
    Windows(const char *name,const char *img_path){
        Window=SDL_CreateWindow(name,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,100,100,SDL_WINDOW_FULLSCREEN_DESKTOP);
        controllo(Window);
        Renderer=SDL_CreateRenderer(Window,-1,SDL_RENDERER_ACCELERATED);
        controllo(Renderer);
        SDL_Surface *Surf=IMG_Load(img_path);
        controllo(Surf);
        Texture=SDL_CreateTextureFromSurface(Renderer,Surf);
        controllo(Texture);
        SDL_FreeSurface(Surf);
        Surf=nullptr;
        SDL_GetWindowSize(Window,(int*)&w,(int*)&h);
    }
    ~Windows(){
        if(Texture!=nullptr)
            SDL_DestroyTexture(Texture);
        Texture=nullptr;
        if(Renderer!=nullptr)
            SDL_DestroyRenderer(Renderer);
        Renderer=nullptr;
        if(Window!=nullptr)
            SDL_DestroyWindow(Window);
        Window=nullptr;
    }
};
struct Button
{
    SDL_Rect Area{0,0,0,0};
    SDL_Renderer *Sfondo{nullptr};
    SDL_Texture *ON{nullptr},*OFF{nullptr};
    SDL_Renderer *Renderer=nullptr;
    bool stato=false;
    Button(const char *ON_path,const char *OFF_path,size_t offsetY,size_t offsetX,size_t h_screen,size_t w_screen,SDL_Renderer *&Renderer_Screen){
        Renderer=Renderer_Screen;
        
        SDL_Surface *Surf=IMG_Load(ON_path);
        controllo(Surf);
        ON=SDL_CreateTextureFromSurface(Renderer,Surf);
        SDL_FreeSurface(Surf);
        Area.w=Surf->w*RIDUTTORE;
        Area.h=Surf->h*RIDUTTORE;

        Surf=IMG_Load(OFF_path);
        controllo(Surf);
        OFF=SDL_CreateTextureFromSurface(Renderer,Surf);
        SDL_FreeSurface(Surf);
        Surf=nullptr;

        Area.y=h_screen/ALTEZZA_S_BOTTONI+offsetY;
        Area.x=w_screen/MARGINE+offsetX;
    }
    ~Button(){
        memset(&Area,0,sizeof(Area));
        SDL_DestroyRenderer(Renderer);
        Renderer=nullptr;
        SDL_DestroyTexture(ON);
        ON=nullptr;
        SDL_DestroyTexture(OFF);
        OFF=nullptr;
    }
    void Render(){
        if(stato){
            SDL_RenderCopy(Renderer,ON,nullptr,&Area);
        }else{
            SDL_RenderCopy(Renderer,ON,nullptr,&Area);
        }
    }
    bool Click(SDL_Point tocco){
        if(tocco.x>Area.x&&tocco.x<Area.x+Area.w&&tocco.y>Area.y&&tocco.y<Area.y+Area.h){
            stato=!stato;
            return true;
        }
    }
};
struct  Text
{
    SDL_Texture *Scritta_T{nullptr};
    SDL_Rect Area={0,0,0,0};
    SDL_Renderer *Renderer=nullptr;
    TTF_Font *font{nullptr};
    Text(const char *Scritta,size_t w_screen,size_t h_screen,SDL_Renderer *&Renderer_screen):Renderer(Renderer_screen){
        font=TTF_OpenFont("font.ttf",h_screen/ALTEZZA_TITOLO);
        controllo(font);
        SDL_Surface *surf=nullptr;
        surf=TTF_RenderText_Solid(font,Scritta,{0,0,0,255});
        controllo(surf);
        Area.h=surf->h;
        Area.w=surf->w;
        Scritta_T=SDL_CreateTextureFromSurface(Renderer,surf);
        SDL_FreeSurface(surf);
        controllo(Scritta_T);
        Area.x=(w_screen-Area.w)/2;
        Area.y=h_screen/MARGINE;
    }
    ~Text(){
        SDL_DestroyTexture(Scritta_T);
        Scritta_T=nullptr;
        memset(&Area,0,sizeof(Area));
    }
    void Render(size_t x,size_t y){
        if(x==0&&y==0){
            SDL_RenderCopy(Renderer,Scritta_T,nullptr,&Area);
        }else{
            Area.x=x;
            Area.y=y;
            SDL_RenderCopy(Renderer,Scritta_T,nullptr,&Area);
        }
    }
    void UpdateText(const char *Scritta){
        SDL_Surface *surf=nullptr;
        surf=TTF_RenderText_Solid(font,Scritta,{0,0,0,255});
        controllo(surf);
        Area.h=surf->h;
        Area.w=surf->w;
        Scritta_T=SDL_CreateTextureFromSurface(Renderer,surf);
        SDL_FreeSurface(surf);
        controllo(Scritta_T);
    }
};

int main() {
    #pragma region SDL
    if(SDL_Init(SDL_INIT_VIDEO)<0){
        std::cerr<<"impossibile inizializzare SDL2";
    }
    if(!IMG_Init(IMG_INIT_JPG)&IMG_INIT_JPG){
        std::cerr<<"impossibile inizializzare SDL_IMG in jpg";
    }
    if(TTF_Init()<0){
        std::cerr<<"impossibile inizializzare TTF";
    }
    Windows finestra("Schermata controllo giostre","sfondo.jpg");
    Text titolo("Schermata controllo giostra",finestra.w,finestra.h,finestra.Renderer);
    Text ClientState("Stato client: ",finestra.w,finestra.h,finestra.Renderer);
    Button avanti("avanti_on.jpg","avanti_off.jpg",titolo.Area.h+finestra.h/MARGINE /*offsetY*/,MARGINE/*offsetX*/,finestra.h,finestra.w,finestra.Renderer),
           indietro("avanti_on.jpg","avanti_off.jpg",titolo.Area.h+finestra.h/MARGINE+avanti.Area.h*2/*offsetY*/,MARGINE/*offsetX*/,finestra.h,finestra.w,finestra.Renderer),
           mode1("avanti_on.jpg","avanti_off.jpg",titolo.Area.h+finestra.h/MARGINE /*offsetY*/,finestra.w-MARGINE-avanti.Area.w/*offsetX*/,finestra.h,finestra.w,finestra.Renderer),
           mode2("avanti_on.jpg","avanti_off.jpg",titolo.Area.h+finestra.h/MARGINE+avanti.Area.h*2/*offsetY*/,finestra.w-MARGINE-avanti.Area.w/*offsetX*/,finestra.h,finestra.w,finestra.Renderer);
    SDL_Point tocco{0,0};
    bool quit=false;
    SDL_Event event;
    #pragma endregion
    #pragma region SOCKET
    int iResult=0;
    int Sock=socket(AF_INET,SOCK_STREAM,0);
    if(Sock==-1){
        std::cerr<<"errore nella creazione del socket\n";
        exit(1);
    }
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    inet_pton(AF_INET,IP,&hint.sin_addr);
    iResult = connect(sock,(sockaddr*)&hint,sizeof(hint));
    if(iResult==-1){
        std::cerr<<"errore nella connesione con il server";
    }
    #pragma endregion
    int size=sizeof(bool)*4+1;
    char *buffer=new char[size];
    while(!quit){
        while(SDL_PollEvent(&event)){
            if(event.type==SDL_QUIT){
                quit=true;
            }
            if(event.type==SDL_FINGERDOWN){
                tocco.x=event.tfinger.x*finestra.w;
                tocco.y=event.tfinger.y*finestra.h;
                if(avanti.Click(tocco)||indietro.Click(tocco)||mode1.Click(tocco)||mode2.Click(tocco)){
                    if(avanti.stato&&!indietro.stato&&(mode1.stato&&!mode2.stato||mode2.stato&&!mode1.stato)){
                        memcpy(buffer,&avanti.stato,sizeof(bool));
                        memcpy(buffer+sizeof(bool),&indi.stato,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*2,&mode1.stato,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*3,&mode2.stato,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*4,'\0',1);
                        iResult=send(Sock,buffer,size,0);
                    }
                    if(indietro.stato&&!avanti.stato&&(mode1.stato&&!mode2.stato||mode2.stato&&!mode1.stato)){
                        memcpy(buffer,&avanti.stato,sizeof(bool));
                        memcpy(buffer+sizeof(bool),&indi.stato,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*2,&mode1.stato,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*3,&mode2.stato,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*4,'\0',1);
                        iResult=send(Sock,buffer,size,0);
                    }
                }
            }
            SDL_RenderClear(finestra.Renderer);
            SDL_RenderCopy(finestra.Renderer,finestra.Texture,nullptr,nullptr);
            titolo.Render(0,0);
            avanti.Render();
            indietro.Render();
            ClientState.Render(finestra.w-ClientState.Area.w-MARGINE,finestra.h-ClientState.Area.h-MARGINE);
            mode1.Render();
            mode2.Render();
            SDL_RenderPresent(finestra.Renderer);
        }
        SDL_Delay(16);
    }
    delete[] buffer;
    close(Sock);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
