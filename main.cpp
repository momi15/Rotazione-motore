#define SDL_MAIN_HANDLED
//librerie SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <fstream>
#include <cstring>
#include <iostream>
#include <cmath>

//librerie linux per la comunicazione 
// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <arpa/inet.h>

//macro per la gestione della finestra
#define ALTEZZA_TITOLO 20
#define ALTEZZA_S_BOTTONI 4
#define Y_BOTTONI 4
#define MARGINE 100
#define RIDUTTORE 0.5
#define ATTESA 3500

//macro per la connesione di rete
#define PORT 27016

//sistema per verificare le variabili
template <typename var>
void controllo(var* variable){
    if(variable==nullptr){
        std::cerr<<"error: "<<SDL_GetError()<<"\n";
        exit(0);
    }
}

//struttura per la gestion della finestra
struct Windows
{
    SDL_Window *Window{nullptr};
    SDL_Renderer *Renderer{nullptr};
    SDL_Texture *Texture{nullptr};
    size_t w{0},h{0};
    //costructor della finestra
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
    //destructor della finestraV
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
//struttura per gestire i bottoni per il motore
struct Button
{
    SDL_Rect Area{0,0,0,0};
    SDL_Renderer *Sfondo{nullptr};
    SDL_Texture *ON{nullptr},*OFF{nullptr},*passivo{nullptr};
    SDL_Renderer *Renderer=nullptr;
    bool stato=true,attivo=true;
    
    //costructor del bottone
    Button(const char *ON_path,const char *OFF_path,const char *Passive_path,size_t offsetY,size_t offsetX,size_t h_screen,size_t w_screen,SDL_Renderer *&Renderer_Screen){
        Renderer=Renderer_Screen;
        //carica l'immagine con il bottone verde
        SDL_Surface *Surf=IMG_Load(ON_path);
        controllo(Surf);
        ON=SDL_CreateTextureFromSurface(Renderer,Surf);
        SDL_FreeSurface(Surf);
        //riduce l'immagine per la macro RIDUTTORE
        Area.w=Surf->w*RIDUTTORE;
        Area.h=Surf->h*RIDUTTORE;

        //carica l'immagine con il bottone rosso
        Surf=IMG_Load(OFF_path);
        controllo(Surf);
        OFF=SDL_CreateTextureFromSurface(Renderer,Surf);
        SDL_FreeSurface(Surf);
        Surf=nullptr;
        
        //carica l'immagine con il bottone grigio
        Surf=IMG_Load(Passive_path);
        controllo(Surf);
        passivo=SDL_CreateTextureFromSurface(Renderer,Surf);
        SDL_FreeSurface(Surf);
        Surf=nullptr;

        Area.y=h_screen/ALTEZZA_S_BOTTONI+offsetY;
        Area.x=w_screen/MARGINE+offsetX;
    }
    
    //destructor del bottone
    ~Button(){
        memset(&Area,0,sizeof(Area));
        SDL_DestroyRenderer(Renderer);
        Renderer=nullptr;
        SDL_DestroyTexture(ON);
        ON=nullptr;
        SDL_DestroyTexture(OFF);
        OFF=nullptr;
    }
    //funzione per renderizzare il bottone
    void Render(){
        //se il bottone è attivo scegle in base allo stato (quindi se è stato schiacciato o meno) quale far vedere
        if(attivo){
            if(stato){
                SDL_RenderCopy(Renderer,ON,nullptr,&Area);
            }else{
                SDL_RenderCopy(Renderer,OFF,nullptr,&Area);
            }
        //se invece non è attivo quindi il tasto è stato disattivato lo fa vedere grigio
        }else{
            SDL_RenderCopy(Renderer,passivo,nullptr,&Area);
        }
    }
    //controllo se è stato cliccato 
    bool Click(SDL_Point tocco,bool alterego,bool &mode1,bool &mode2){
        //se il click era nell'area del pulsante e il pulsante opposto non è stato schiacciato il pulsante fa vedere la texture rossa
        if(tocco.x>Area.x&&tocco.x<Area.x+Area.w&&tocco.y>Area.y&&tocco.y<Area.y+Area.h&&alterego){
            stato=false;
            if(mode2){
                mode1=false;
            }
            if(!attivo) attivo=true;
            return true;
        }
        //se invece l'alterego è stato schiacciato il tasto si disattiva
        if(!alterego){
            attivo=false;
        }
        //se il click non è nell'area del pulsante allora returna false
        return false;
    }
    bool Click2(SDL_Point tocco){
        //se il click era nell'area del pulsante e il pulsante opposto non è stato schiacciato il pulsante fa vedere la texture rossa
        if(tocco.x>Area.x&&tocco.x<Area.x+Area.w&&tocco.y>Area.y&&tocco.y<Area.y+Area.h){
            return true;
        }
        return false;
    }
};
struct RideButton
{
    SDL_Texture *Scritta_t{nullptr},*Sfondo_t{nullptr};
    SDL_Rect Area_sfondo{0,0,0,0};
    SDL_FRect Area_txt{.0f,.0f,.0f,.0f};
    SDL_Renderer *Renderer{nullptr};
    TTF_Font *font{nullptr};
    RideButton(const char* Name,const char* Background_path,size_t w_screen,size_t h_screen,SDL_Renderer *&Renderer_screen,size_t TextSize,size_t offsetY,size_t ProporzioneMargine){
        Renderer=Renderer_screen;
        font=TTF_OpenFont("font/font.ttf",h_screen/ALTEZZA_TITOLO*TextSize);
        controllo(font);

        SDL_Surface *surf=nullptr;
        surf=TTF_RenderText_Solid(font,Name,{0,0,0,255});
        controllo(surf);

        Area_txt.h=surf->h;
        Area_txt.w=surf->w;

        Scritta_t=SDL_CreateTextureFromSurface(Renderer,surf);
        SDL_FreeSurface(surf);
        controllo(Scritta_t);

        surf=IMG_Load(Background_path);
        controllo(surf);
        Area_sfondo.h=surf->h;
        Area_sfondo.w=surf->w;
        Sfondo_t=SDL_CreateTextureFromSurface(Renderer,surf);
        SDL_FreeSurface(surf);
        controllo(Sfondo_t);
        size_t costante=Area_sfondo.w/Area_sfondo.h;

        Area_sfondo.w=Area_txt.w+MARGINE*ProporzioneMargine;
        Area_sfondo.h=Area_txt.h+MARGINE*ProporzioneMargine;

        Area_sfondo.x=(w_screen-Area_sfondo.w)/2;
        Area_sfondo.y=offsetY+MARGINE;

        Area_txt.x=Area_sfondo.x+MARGINE*ProporzioneMargine/2;
        Area_txt.y=Area_sfondo.y+MARGINE*ProporzioneMargine/2;
    }
    ~RideButton(){
        SDL_DestroyTexture(Scritta_t);
        Scritta_t=nullptr;
        SDL_DestroyTexture(Sfondo_t);
        Sfondo_t=nullptr;
        SDL_DestroyRenderer(Renderer);
        Renderer=nullptr;
    }
    void Render(){
        SDL_RenderCopy(Renderer,Sfondo_t,nullptr,&Area_sfondo);
        SDL_RenderCopyF(Renderer,Scritta_t,nullptr,&Area_txt);
    }
    //controllo se è stato cliccato 
    bool Click(SDL_Point tocco){
        //se il click era nell'area del pulsante e il pulsante opposto non è stato schiacciato il pulsante fa vedere la texture rossa
        if(tocco.x>Area_sfondo.x&&tocco.x<Area_sfondo.x+Area_sfondo.w&&tocco.y>Area_sfondo.y&&tocco.y<Area_sfondo.y+Area_sfondo.h){
            return true;
        }
        return false;
    }
};

//struttura per gestire le scritte
struct  Text
{
    SDL_Texture *Scritta_T{nullptr};
    SDL_Rect Area={0,0,0,0};
    SDL_Renderer *Renderer=nullptr;
    TTF_Font *font{nullptr};
    //costructor della struttura
    Text(const char *Scritta,size_t w_screen,size_t h_screen,SDL_Renderer *&Renderer_screen,size_t TextSize):Renderer(Renderer_screen){
        //la grandezza del testo va in base alla grandezza dello schermo e grazie alla variabile TextSize si può fare una scritta più grande rispetto all'altra
        font=TTF_OpenFont("font/font.ttf",h_screen/ALTEZZA_TITOLO*TextSize);
        controllo(font);

        //superficie su sui viene fatta la scritta
        SDL_Surface *surf=nullptr;
                                            //colore scritta
        surf=TTF_RenderText_Solid(font,Scritta,{0,0,0,255});
        controllo(surf);

        Area.h=surf->h;
        Area.w=surf->w;
        //conversione da superficie a texture per le prestazioni
        Scritta_T=SDL_CreateTextureFromSurface(Renderer,surf);
        SDL_FreeSurface(surf);
        controllo(Scritta_T);
        //fa la scritta perfettamente centrata nella finestra
        Area.x=(w_screen-Area.w)/2;
        //in alto lascia uno spazietto che è in proporzione allo schermo
        Area.y=h_screen/MARGINE;
    }
    //destructor della struttura
    ~Text(){
        SDL_DestroyTexture(Scritta_T);
        Scritta_T=nullptr;
        memset(&Area,0,sizeof(Area));
    }
    //funzione per renderizzare
    void Render(){
        SDL_RenderCopy(Renderer,Scritta_T,nullptr,&Area);
    }
    //funzione per aggiornare la scritta
    void UpdateText(const char *Scritta,size_t w_screen,size_t h_screen){
        SDL_Surface *surf=nullptr;
        surf=TTF_RenderText_Solid(font,Scritta,{0,0,0,255});
        controllo(surf);
        Area.h=surf->h;
        Area.w=surf->w;
        Area.x=(w_screen-Area.w)/2;
        //in alto lascia uno spazietto che è in proporzione allo schermo
        Area.y=h_screen/MARGINE;
        Scritta_T=SDL_CreateTextureFromSurface(Renderer,surf);
        SDL_FreeSurface(surf);
        controllo(Scritta_T);
    }
};
int main() {
    #pragma region SDL
    //inizializzazione di SDL 
    if(SDL_Init(SDL_INIT_VIDEO)<0){
        std::cerr<<"impossibile inizializzare SDL2";
    }
    //inizializzazione di SDL_image per poter caricare i jpg
    if(!IMG_Init(IMG_INIT_JPG)&IMG_INIT_JPG){
        std::cerr<<"impossibile inizializzare SDL_IMG in jpg";
    }
    //inizializzazione di SDL_image per poter caricare i png
    if(!IMG_Init(IMG_INIT_PNG)&IMG_INIT_PNG){
        std::cerr<<"impossibile inizializzare SDL_IMG in png";
    }
    //inizializzazione di SDL_ttf per poter fare le scritte
    if(TTF_Init()<0){
        std::cerr<<"impossibile inizializzare TTF";
    }
    SDL_SetCursor(SDL_DISABLE);
    enum Schermate{
        Giostre,
        Motore,
        Video,
    };
    uint8_t CurrentScreen=Schermate::Giostre;
    //definizione struttura per la finestra
    Windows finestra("Schermata controllo giostre","img/sfondo.jpg");
    //definizioni delle varie scritte
    Text titolo("Schermata controllo giostra",finestra.w,finestra.h,finestra.Renderer,2),
         ClientState("Stato motore: Disconnesso",finestra.w,finestra.h,finestra.Renderer,1),
         attesa("attendere...",finestra.w,finestra.h,finestra.Renderer,1);
    //definizioni dei vari bottoni schermata Motore
    Button avanti("img/avanti_on.jpg"/*bottone non schiacciato*/,"img/avanti_off.jpg"/*bottone schiacciato*/,"img/avanti_passive.jpg"/*bottone disattivato*/,titolo.Area.h+finestra.h/MARGINE /*offsetY*/                 ,MARGINE/*offsetX*/                         ,finestra.h,finestra.w,finestra.Renderer),
           indietro("img/avanti_on.jpg"/*bottone non schiacciato*/,"img/avanti_off.jpg"/*bottone schiacciato*/,"img/avanti_passive.jpg"/*bottone disattivato*/,titolo.Area.h+finestra.h/MARGINE+avanti.Area.h*2/*offsetY*/,MARGINE/*offsetX*/                         ,finestra.h,finestra.w,finestra.Renderer),
           mode1("img/avanti_on.jpg"/*bottone non schiacciato*/,"img/avanti_off.jpg"/*bottone schiacciato*/,"img/avanti_passive.jpg"/*bottone disattivato*/,titolo.Area.h+finestra.h/MARGINE /*offsetY*/                  ,finestra.w-MARGINE-avanti.Area.w/*offsetX*/,finestra.h,finestra.w,finestra.Renderer),
           mode2("img/avanti_on.jpg"/*bottone non schiacciato*/,"img/avanti_off.jpg"/*bottone schiacciato*/,"img/avanti_passive.jpg"/*bottone disattivato*/,titolo.Area.h+finestra.h/MARGINE+avanti.Area.h*2/*offsetY*/   ,finestra.w-MARGINE-avanti.Area.w/*offsetX*/,finestra.h,finestra.w,finestra.Renderer),
           close("img/close.png","img/close.png","img/close.png",0,0,finestra.h,finestra.w,finestra.Renderer),
           stop("img/avanti_on.jpg"/*bottone non schiacciato*/,"img/avanti_on.jpg"/*bottone schiacciato*/,"img/avanti_passive.jpg"/*bottone disattivato*/,0,0,finestra.h,finestra.w,finestra.Renderer),
           video_b("img/avanti_on.jpg","img/avanti_on.jpg","img/avanti_passive.jpg",0,0,finestra.h,finestra.w,finestra.Renderer);
    std::string giostre_s[3],leggi;
    std::ifstream file("file_modificabili/giostre.txt");
    for(uint8_t i=0;std::getline(file,leggi);++i){
        giostre_s[i]=leggi;
    }
           //definizione dei vari bottoni con i nomi delle giostre
    RideButton Giostra1(giostre_s[0].c_str(),"img/sfondo_pulsante_giostra1.jpg",finestra.w,finestra.h,finestra.Renderer,1,MARGINE,1),
               Giostra2(giostre_s[1].c_str(),"img/sfondo_pulsante_giostra1.jpg",finestra.w,finestra.h,finestra.Renderer,1,Giostra1.Area_sfondo.h+Giostra1.Area_sfondo.y,1),
               Giostra3(giostre_s[2].c_str(),"img/sfondo_pulsante_giostra1.jpg",finestra.w,finestra.h,finestra.Renderer,1,Giostra2.Area_sfondo.h+Giostra2.Area_sfondo.y,1);
    for(size_t i=0;i<3;++i){
        giostre_s[i].clear();
    }
    leggi.clear();
    //imposto la posizione del bottone di stop
    stop.Area.x=finestra.w/2-stop.Area.w/2;
    stop.Area.y=finestra.h/2;
    //imposto la posizione del pulsante di chiusura
    close.Area.x=MARGINE/2;
    close.Area.y=MARGINE/2;
    //imposto la posizione del pulsante del video
    video_b.Area.y=finestra.h-video_b.Area.h;
    video_b.Area.x=0;
    //imposto la posizione scritta dello stato del motore
    ClientState.Area.x=finestra.w-ClientState.Area.w-MARGINE;
    ClientState.Area.y=finestra.h-ClientState.Area.h-MARGINE;
    //imposto la posizione della scritta dell'attesa
    attesa.Area.x=(finestra.w-attesa.Area.w)/2;
    attesa.Area.y=stop.Area.y+stop.Area.h+MARGINE;

    SDL_Point tocco{0,0};
    bool quit=false,NewPage=true;
    SDL_Event event;

    #pragma endregion
    #pragma region SOCKET
    //lettura del file ip.txt in cui è scritto l'ip dell'ESP32
   file.open("file_modificabili/ip.txt");
    std::string ip;
    std::getline(file,ip);
    std::cout<<ip<<"\n";
    file.close();
    //creo il socket per poter fare la comunicazione con l'ESP32
    /*
    int iResult=0;
    int Sock=socket(AF_INET,SOCK_STREAM,0);
    if(Sock==-1){
        std::cerr<<"errore nella creazione del socket\n";
        exit(1);
    }
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    inet_pton(AF_INET,ip.c_str(),&hint.sin_addr);
    iResult = connect(sock,(sockaddr*)&hint,sizeof(hint));
    if(iResult==-1){
        std::cerr<<"errore nella connesione con il server";
    }*/
   //creazione del buffer che conterà i dati da mandare
   int size=sizeof(bool)*5+1;
   char *buffer=new char[size];
   //variabile che gestirà il quando mandare i dati quando no
   bool invia=false;
   //variabile che serve nel caso i dati non siano stati inviati non aggiorni l'UI
   bool inviato=false;
   //variabili che verranno inviate all'ESP32
   bool BuffAvv=false,BuffIndi=false,BuffMode1=false,BuffMode2=false,allarme=false;
   #pragma endregion
   //parte dall'indirizzo della memoria del buffer e da li mette i dati delle varie variabili
    memcpy(buffer,&BuffAvv,sizeof(bool));
    memcpy(buffer+sizeof(bool),&BuffIndi,sizeof(bool));
    memcpy(buffer+sizeof(bool)*2,&BuffMode1,sizeof(bool));
    memcpy(buffer+sizeof(bool)*3,&BuffMode2,sizeof(bool));
    memcpy(buffer+sizeof(bool)*4,&allarme,sizeof(bool));
    memcpy(buffer+sizeof(bool)*5,"\0",1);
    // iResult=send(Sock,buffer,size,0);
    //  //sistema per verificare lo stato con il motore
    // if(iResult==-1){
    //     std::cerr<<"impossibile mandare il messaggio\n";
    //    ClientState.UpdateText("Stato motore: Sconnesso");
    //    inviato=false;
    // }
    //else{
    //    ClientState.UpdateText("Stato motore: Connesso");                            
    //    inviato=true;
    //}
    //è nell loop fi quando non si chiude la finestra
    uint8_t tipoGiostra;
    while(!quit){
        while(SDL_PollEvent(&event)){
            if(event.type==SDL_QUIT){
                quit=true;
            }
            //controlla se il tasto del mouse si alza
            if(event.type==SDL_MOUSEBUTTONUP){
                //prende la posizione del mouse
                SDL_GetMouseState(&tocco.x,&tocco.y);
                //se è stato premuto il pulsante di chiusura e chiude
                if(close.Click(tocco,true,mode1.stato,mode2.stato)){
                    //imposta tutti i pulsanti sul verde,gli disativa e imposta i dati che verranno inviati tutti su false
                    avanti.stato=true;
                    avanti.attivo=false;
                    BuffAvv=false;

                    indietro.stato=true;
                    indietro.attivo=false;
                    BuffIndi=false;
                            
                    mode1.stato=true;
                    mode1.attivo=false;
                    BuffMode1=false;
                            
                    mode2.stato=true;
                    mode2.attivo=false;
                    BuffMode2=false;

                    video_b.attivo=false;
                    stop.attivo=false;
                    //attiva l'allarme che aprirà il relè per scaricare il condensatore
                    allarme=true;

                    //prepara il buffer
                    memcpy(buffer,&BuffAvv,sizeof(bool));
                    memcpy(buffer+sizeof(bool),&BuffIndi,sizeof(bool));
                    memcpy(buffer+sizeof(bool)*2,&BuffMode1,sizeof(bool));
                    memcpy(buffer+sizeof(bool)*3,&BuffMode2,sizeof(bool));
                    memcpy(buffer+sizeof(bool)*4,&allarme,sizeof(bool));
                    memcpy(buffer+sizeof(bool)*5,"\0",1);
                    //imposta l'allarme su false tanto verrà inviata la variabile buffer
                    allarme=false;
                    // iResult=send(Sock,buffer,size,0);
                    // if(iResult==-1){
                    //     std::cerr<<"impossibile mandare il messaggio\n";
                    //    ClientState.UpdateText("Stato motore: Sconnesso");
                    //    inviato=false;
                    // }
                    //else{
                    //    ClientState.UpdateText("Stato motore: Connesso");                            
                    //    inviato=true;
                    //}
                    //renderizza i pulsanti disattivati
                    SDL_RenderClear(finestra.Renderer);
                    SDL_RenderCopy(finestra.Renderer,finestra.Texture,nullptr,nullptr);
                    titolo.Render();
                    close.Render();
                    switch (CurrentScreen)
                    {
                        case Giostre:
                        Giostra1.Render();
                        Giostra2.Render();
                        Giostra3.Render();
                        attesa.Area.y=Giostra3.Area_sfondo.x+Giostra3.Area_sfondo.h+finestra.h/MARGINE*3;
                        break;
                        
                        case Motore:
                        avanti.Render();
                        indietro.Render();
                        ClientState.Render();
                        mode1.Render();
                        mode2.Render();
                        close.Render();
                        stop.Render();
                        video_b.Render();
                        attesa.Area.y=stop.Area.y+stop.Area.h+MARGINE;
                        break;
                    }
                    attesa.Render();
                    SDL_RenderPresent(finestra.Renderer);
                    SDL_Delay(3500);
                    quit=true;
                }
                //controlla se è stato premuto il pulsante di stop se si attiva la procedura
                switch (CurrentScreen)
                {
                    case Giostre:{
                        if(Giostra1.Click(tocco)){
                            tipoGiostra=1;
                            CurrentScreen=Motore;
                            NewPage=true;
                        }
                        if(Giostra2.Click(tocco)){
                            tipoGiostra=2;
                            CurrentScreen=Motore;
                            NewPage=true;
                        }
                        if(Giostra3.Click(tocco)){
                            tipoGiostra=3;
                            CurrentScreen=Motore;
                            NewPage=true;
                        }
                        break;
                    }
                    case Video:{
                    }
                    case Motore:{
                        if(video_b.Click2(tocco)){
                            CurrentScreen=Video;
                            NewPage=true;
                        }
                        //se era sul pulsante di stop attiva la procedura
                        if(stop.Click(tocco,true,mode1.stato,mode2.stato)){
                            //imposta tutti i pulsanti sul verde,gli disativa e imposta i dati che verranno inviati tutti su false
                            avanti.stato=true;
                            avanti.attivo=false;
                            BuffAvv=false;

                            indietro.stato=true;
                            indietro.attivo=false;
                            BuffIndi=false;
                            
                            mode1.stato=true;
                            mode1.attivo=false;
                            BuffMode1=false;
                            
                            mode2.stato=true;
                            mode2.attivo=false;
                            BuffMode2=false;

                            video_b.attivo=false;
                            stop.attivo=false;
                            //attiva l'allarme che aprirà il relè per scaricare il condensatore
                            allarme=true;

                            //prepara il buffer
                            memcpy(buffer,&BuffAvv,sizeof(bool));
                            memcpy(buffer+sizeof(bool),&BuffIndi,sizeof(bool));
                            memcpy(buffer+sizeof(bool)*2,&BuffMode1,sizeof(bool));
                            memcpy(buffer+sizeof(bool)*3,&BuffMode2,sizeof(bool));
                            memcpy(buffer+sizeof(bool)*4,&allarme,sizeof(bool));
                            memcpy(buffer+sizeof(bool)*5,"\0",1);
                            //imposta l'allarme su false tanto verrà inviata la variabile buffer
                            allarme=false;
                            // iResult=send(Sock,buffer,size,0);
                            // if(iResult==-1){
                            //     std::cerr<<"impossibile mandare il messaggio\n";
                            //    ClientState.UpdateText("Stato motore: Sconnesso");
                            //    inviato=false;
                            // }
                            //else{
                            //    ClientState.UpdateText("Stato motore: Connesso");                            
                            //    inviato=true;
                            //}
                            //renderizza i pulsanti disattivati
                            SDL_RenderClear(finestra.Renderer);
                            SDL_RenderCopy(finestra.Renderer,finestra.Texture,nullptr,nullptr);
                            titolo.Render();
                            avanti.Render();
                            indietro.Render();
                            ClientState.Render();
                            mode1.Render();
                            mode2.Render();
                            close.Render();
                            stop.Render();
                            video_b.Render();
                            attesa.Render();
                            SDL_RenderPresent(finestra.Renderer);
                            //aspetta che il condesatore si scarichi
                            SDL_Delay(ATTESA);
                            //attiva tutti i pulsanti
                            avanti.attivo=true;
                            indietro.attivo=true;
                            mode1.attivo=true;
                            mode2.attivo=true;
                            stop.attivo=true;
                            video_b.attivo=true;
                        }
                        //controllo se è stato schiacciato uno dei 4 pulsanti
                        if(avanti.Click(tocco,indietro.stato,mode1.stato,mode2.stato)||indietro.Click(tocco,avanti.stato,mode1.stato,mode2.stato)){
                            //controllo se è stato schiacciato solo 1 dei 2 pulsanti reciproci se si allora invia i dati
                            if((!avanti.stato&&indietro.stato||!indietro.stato&&avanti.stato)&&(mode1.stato&&!mode2.stato||mode2.stato&&!mode1.stato)){
                                invia=true;
                            }
                        }
                        //controllo se è stato schiacciato uno dei 4 pulsanti
                        if(mode2.Click(tocco,mode1.stato,mode1.stato,mode2.stato)||mode1.Click(tocco,mode2.stato,mode1.stato,mode2.stato)){
                            if((!avanti.stato&&indietro.stato||!indietro.stato&&avanti.stato)&&(!mode1.stato&&mode2.stato||!mode2.stato&&mode1.stato)){
                                invia=true;
                            }
                        }
                        //calcolo se un pulsante deve essere attivo o meno
                        avanti.attivo=indietro.stato;
                        indietro.attivo=avanti.stato;
                        mode1.attivo=mode2.stato;
                        mode2.attivo=mode1.stato;
                        //se è stato premuto uno dei 2 pulsanti imposta in automatico la velocità più lenta
                        if((!avanti.stato||!indietro.stato)&&mode2.stato){
                            mode1.stato=false;
                        }
                        //invia i dati al ESP32
                    if(invia){
                        //calcola il valore delle variabili che verranno inviate
                        BuffAvv=!avanti.stato;
                        BuffIndi=!indietro.stato;
                        if(BuffAvv&&!mode1.stato||!BuffAvv){
                            BuffMode1=false;
                        }else if(BuffAvv&&!mode2.stato){
                            BuffMode1=true;
                        }
                        if(BuffIndi&&!mode1.stato||!BuffIndi){
                            BuffMode2=false;
                        }else if(BuffIndi&&!mode2.stato){
                            BuffMode2=true;
                        }
                        BuffMode2!=mode2.stato;
                        //prepare il buffer
                        memcpy(buffer,&BuffAvv,sizeof(bool));
                        memcpy(buffer+sizeof(bool),&BuffIndi,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*2,&BuffMode1,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*3,&BuffMode2,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*4,&allarme,sizeof(bool));
                        memcpy(buffer+sizeof(bool)*5,"\0",1);
                        std::cout<<"avanti: "<<BuffAvv<<"\t\t velocita': "<<BuffMode1<<"\n";
                        std::cout<<"indietro: "<<BuffIndi<<"\t\t velocita': "<<BuffMode2<<"\n\n\n\n";
                        // iResult=send(Sock,buffer,size,0);
                        // if(iResult==-1){
                            //     std::cerr<<"impossibile mandare il messaggio\n";
                            //    ClientState.UpdateText("Stato motore: Sconnesso");
                            //    inviato=false;
                            // }
                            //else{
                                //    ClientState.UpdateText("Stato motore: Connesso");                            
                                //    inviato=true;
                            //}
                             //se i dati sono stati inviati allora aggiorna l'UI
                            invia=false;
                        }
                        if(inviato){
                            if(!avanti.stato){
                                BuffAvv=true;
                                if(!mode1.stato)
                                BuffMode1=false;
                                if(!mode2.stato)
                                BuffMode1=true;
                            }
                            if(!indietro.stato){
                                BuffAvv=true;
                                if(!mode1.stato)
                                BuffMode2=false;
                                if(!mode2.stato)
                                BuffMode2=true;
                            }
                        }
                        break;
                    }
                }
            }
        }
        //aggiorna la schermata
        SDL_RenderClear(finestra.Renderer);
        if(CurrentScreen!=Video)
        SDL_RenderCopy(finestra.Renderer,finestra.Texture,nullptr,nullptr);
        switch(CurrentScreen){
            case Giostre:{
                if(NewPage){
                    titolo.UpdateText("Scegli la giostra che vuoi",finestra.w,finestra.h);
                    NewPage=false;
                }
                titolo.Render();
                Giostra1.Render();
                Giostra2.Render();
                Giostra3.Render();
                break;
            }
            case Motore:{
                if(NewPage){
                    titolo.UpdateText("Schermata controllo giostra",finestra.w,finestra.h);
                    NewPage=false;
                }
                titolo.Render();
                avanti.Render();
                indietro.Render();
                ClientState.Render();
                mode1.Render();
                mode2.Render();
                video_b.Render();
                stop.Render();
                break;
            }
        }
        close.Render();        
        SDL_RenderPresent(finestra.Renderer);
        //fa girare tutti a 60fps 1000/60=16
        SDL_Delay(16);
    }
    //libera memoria
    delete[] buffer;
//  close(Sock);
    //deinizializza le robe di SDL
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}