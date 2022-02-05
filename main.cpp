#include <iostream>
#include<bits/stdc++.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfx.h>
#include <string>
#include <math.h>
#include <vector>
#include <fstream>
#include <SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <cstdlib>
#include <ctime>

using namespace std;


void textRGBA(SDL_Renderer*m_renderer, int x, int y, const char* text,int useless,int f_size,int r, int g, int b, int a);

Mix_Chunk*kickEffect;
Mix_Chunk*bounceEffect;
Mix_Chunk*starSound;
Mix_Music*bgMusic;

SDL_Renderer * m_renderer;
SDL_Texture * background=SDL_CreateTexture(m_renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,1280,800);
const int W = 1280;
const int H = 800;
int sl_x = W / 2+ 300;

//global stat
int stat = 0;

double scaling_factor= 1.2;
bool beginningOfNewGameSeries=true;
void fireballSplashScreen ();

// class definition
////address of chunks to free
//struct effectChunks
//{
//    Mix_Chunk** bounce;
//    Mix_Chunk** kick;
//
//    bool freeKickChunk=false;
//    bool freebounceChunk=false;
//};

struct setting
{
    int bg;
    int head;
    int body;
    int ball;
    int sec;
};

class timer
{
private:
    time_t GameStartTime;
    time_t newGameStart;

    time_t p_time;


public:
    timer()
    {
        time(&GameStartTime);
    }

    void setGameStart()
    {
        time(&newGameStart);
    }

    int getLocalElapsedTime ()
    {
        time_t now;
        time(&now);

        return (now-newGameStart);
    }

    int getTotalElapsedTime ()
    {
        time_t now;
        time(&now);

        return (now-GameStartTime);
    }

    void pauseTime()
    {
        time(&p_time);
    }

    void resumeTime()
    {
        time_t now;
        time(&now);

        GameStartTime+=now-p_time;
    }

};

struct motions
{
    int LEFT;
    int RIGHT;
    int JUMP;
    int POW; //special power
    int KICK;
};

class star
{
private:
    int xpos;
    int ypos;
    int framesPassed=0;
    bool inGame=false;
    time_t t_start;

public:

    int getX()
    {
       return xpos;
    }

    int getY()
    {
       return ypos;
    }

    void render(bool magnify=false)
    {
        SDL_Texture*s_t;
        SDL_Rect s_rect{xpos-11,ypos-10,22,20};
        if (magnify)
        {
            s_rect.w*=1.3;
            s_rect.h*=1.3;
            s_rect.x=xpos-s_rect.w/2;
            s_rect.y=ypos-s_rect.h/2;
        }

        s_t=IMG_LoadTexture(m_renderer,"./Textures/star.png");

        SDL_RenderCopy(m_renderer,s_t,NULL,&s_rect);

        SDL_DestroyTexture(s_t);
    }

    bool isInGame()
    {
        return inGame;
    }

    bool evenFrame()
    {
        return(framesPassed%2==0);
    }

    void die()
    {
        framesPassed=0;
        inGame=false;
    }

    //0 for regular, 1 for flickering, -1 for dead
    int lifeSpan()
    {
        time_t now;
        time(&now);

        int duration=now-t_start;
        if (duration<=1)
        {
            framesPassed++;
            return 0;
        }
        else if (duration<=2)
        {
            framesPassed++;
            return 1;
        }
        else
        {
            die();
            return -1;
        }
    }

    void isBorn(int x, int y)
    {
        time(&t_start);
        inGame=true;
        xpos=x;
        ypos=y;

        render();
    }

};

class bomb
{
private:
    int xpos;
    int ypos;
    int framesPassed=0;
    bool inGame=false;
    time_t t_start;

public:

    int getX()
    {
       return xpos;
    }

    int getY()
    {
       return ypos;
    }

    void render(bool magnify=false)
    {
        SDL_Texture*s_t;
        SDL_Rect s_rect{xpos-10,ypos-10,20,20};
        if (magnify)
        {
            s_rect.w*=1.3;
            s_rect.h*=1.3;
            s_rect.x=xpos-s_rect.w/2;
            s_rect.y=ypos-s_rect.h/2;
        }

        s_t=IMG_LoadTexture(m_renderer,"./Textures/bomb.png");

        SDL_RenderCopy(m_renderer,s_t,NULL,&s_rect);

        SDL_DestroyTexture(s_t);
    }

    bool isInGame()
    {
        return inGame;
    }

    bool evenFrame()
    {
        return(framesPassed%2==0);
    }

    void die()
    {
        framesPassed=0;
        inGame=false;
    }

    //0 for regular, 1 for flickering, -1 for dead
    int lifeSpan()
    {
        time_t now;
        time(&now);

        int duration=now-t_start;
        if (duration<=1)
        {
            framesPassed++;
            return 0;
        }
        else if (duration<=2)
        {
            framesPassed++;
            return 1;
        }
        else
        {
            die();
            return -1;
        }
    }

    void isBorn(int x, int y)
    {
        time(&t_start);
        inGame=true;
        xpos=x;
        ypos=y;

        render();
    }

};


class player
{
private:

    const Uint8 *keyboard_state_array = SDL_GetKeyboardState(NULL);
    int r_speed=20; //sprint speed
    int default_jump=53;// default speed
    int j_speed=default_jump; //jump speed
    int gravity=7;

    //powerUp boost from eating stars
    int starBoost=0;

    int frameCount=0;
    int player_num;
    bool jumping =false;
    struct motions controls;

    char specialPower;

    time_t powerUpStartTime;
    time_t localTimeStart;
    time_t stunStartTime;

    char skin_code;
    char jersey_code;

    void initControls ()
    {
        if (player_num==1)
            controls = {SDL_SCANCODE_A,SDL_SCANCODE_D, SDL_SCANCODE_W,SDL_SCANCODE_LSHIFT,SDL_SCANCODE_S};
        else
            controls= {SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,SDL_SCANCODE_UP,SDL_SCANCODE_RSHIFT,SDL_SCANCODE_DOWN};
    }

    void loadHeadTexture(SDL_Texture ** skin)
    {
        string skn="./Textures/Player/Head/";
        skn.push_back(skin_code);
        skn+=".png";
        *skin=IMG_LoadTexture(m_renderer,skn.c_str());
    }

    void loadJerseyTexture( SDL_Texture ** jersey)
    {
        string jrs="./Textures/Player/Body/";
        jrs.push_back(jersey_code);
        jrs+=".png";
        *jersey=IMG_LoadTexture(m_renderer,jrs.c_str());

    }

    void powerInit(char c)
    {
        switch (c)
        {
        case ('6'):
            specialPower='c'; //clone
            break;
        case ('B'):
            specialPower='c'; //clone
            break;

        case ('7'):
            specialPower='p'; //punch
            break;
        case ('1'):
            specialPower='p'; //punch
            break;

        case ('8'):
            specialPower='i'; //invisible ball
            break;
        case ('C'):
            specialPower='i'; //invisible ball
            break;

        case ('4'):
            specialPower='k'; //kick fireball
            break;
        case ('3'):
            specialPower='k'; //kick fireball
            break;

        case ('5'):
            specialPower='h'; //head fireball
            break;
        case ('A'):
            specialPower='h'; //head fireball
            break;

        default:
            //thief
            string str="cpikh";
            specialPower=str[rand()%(str.length())];
            break;

        }
    }

    void endJump()
    {
        jumping=false;
        j_speed=default_jump;
        ypos=groundheight;
    }

    void clearActivity()
    {
        activity[0]=true;
        for (int i=1; i<6; i++)
            activity[i]=false;
    }


public:

    int head_r=60*scaling_factor; //head radius
    int body_w=56*scaling_factor;
    int body_h=120*scaling_factor;

    //position of sprite's head
    int xpos=120;
    int groundheight=H-body_h-head_r;
    int ypos=groundheight;

    void lowerGravity()
    {
        gravity/=2;
    }

    void stun()
    {
        activity[5]=true;
        frameCount=0;

        time(&stunStartTime);
    }

    void resetFrameCount()
    {
        frameCount=0;
    }

    int stunDuration()
    {
        time_t now;
        time(&now);

        SDL_Rect s_rect;
        SDL_Texture* s_texture;

        string address="./Textures/images/";
        address.push_back(frameCount%10+'0');
        address+=".png";

        s_rect.w=176*scaling_factor;
        s_rect.h=69*scaling_factor;
        s_rect.y=H-body_h-head_r-s_rect.h;
        s_rect.x=xpos-s_rect.w/2;

        s_texture=IMG_LoadTexture(m_renderer, address.c_str());
        SDL_RenderCopy(m_renderer,s_texture,NULL,&s_rect);
        frameCount++;

        SDL_DestroyTexture(s_texture);

        return (now-stunStartTime);
    }

    int getPlayerNum ()
    {
        return player_num;
    }

    void setPowerUpStartTime()
    {
        time(&powerUpStartTime);
    }

    int powerUpDuration()
    {
        time_t now;
        time(&now);

        return (now-powerUpStartTime);
    }

    bool activity[6]; ///0: still, 1: run left, 2:run right, 3: kick, 4: powerUp, 5: dazed

    bool inGame=false;

    bool isJumping()
    {
        return jumping;
    }

    void hasJumped()
    {
        jumping=true;
        j_speed=default_jump;
    }

    int max_h()
    {
        return ypos+body_h+head_r;
    }

    int min_h()
    {
        return ypos-head_r;
    }

    int min_w()
    {
        return xpos-body_w/2;
    }

    int max_w()
    {
        return xpos+body_w/2;
    }

    player()
    {
        inGame=false;
    }

    ~player()
    {}

    player (char skn_code, char jersey_color, int num )
    {
        skin_code=skn_code;
        jersey_code=jersey_color;

        clearActivity();
        inGame=true;
        player_num=num;

        if (player_num==2)
            xpos=W-xpos;

//        drawPlayer();
        initControls();
        powerInit(skn_code);

        time(&localTimeStart);
    }

    void setLocalTime()
    {
        time(&localTimeStart);
    }

    int localElapsedTime ()
    {
        time_t now;
        time(&now);

        return (now-localTimeStart);
    }

    int getXVel()
    {
        if (activity[0])
            return 0;

        else if (activity[1])
            return (-r_speed);

        else if (activity[2])
            return r_speed;
    }

    int getYVel()
    {
        if (jumping)
            return j_speed;

        return 0;
    }

    char getSpecialPower()
    {
        return specialPower;
    }

    void intersectsWith(player *Opponent)
    {
        //colliding with opponent from the right
        if (this->xpos-(*Opponent).xpos<=body_w && this->xpos-(*Opponent).xpos>0 && (*Opponent).ypos-this->ypos<body_h && (*Opponent).ypos-this->ypos>=0)
        {
            endJump();
            this->xpos=(*Opponent).xpos+body_w+1;
        }

        //colliding with opponent from the left
        else if ((*Opponent).xpos-this->xpos<=body_w && (*Opponent).xpos-this->xpos>0 && (*Opponent).ypos-this->ypos<body_h && (*Opponent).ypos-this->ypos>=0)
        {
            endJump();
            this->xpos=(*Opponent).xpos-body_w-1;
        }
    }

    bool powerBar(int scores[])
    {
        int num_bars;
        int timeFactor=0.2*localElapsedTime()+starBoost;
        Uint32 color;

        if (player_num==1)
        {
            num_bars=min(10,min(4,scores[1]/(scores[0]+1))+int(timeFactor));

            color=(num_bars!=10 || activity[4])? 0xff00ff00:(localElapsedTime())%2==0 ? 0xff00ffff:0xff00aaff;

            roundedBoxColor(m_renderer,15,15,465,60,22,0xff000000);

            if (num_bars!=0)
                roundedBoxColor(m_renderer,23,21,23+43.5*num_bars,54,18,color);
        }

        else
        {
            num_bars=min(10,min(4,scores[0]/(scores[1]+1))+int(timeFactor));

            color=(num_bars!=10 || activity[4])? 0xff00ff00:(localElapsedTime())%2==0 ? 0xff00ffff:0xff00aaff;

            roundedBoxColor(m_renderer,W-465,15,W-15,60,22,0xff000000);

            if (num_bars!=0)
                roundedBoxColor(m_renderer,W-43.5*num_bars-23,21,W-23,54,18, color);
        }

        if (num_bars==10)
            return true;

        return false;

    }

    void isOutOfBounds()
    {
        if (player_num==1)
        {
            if (min_w()<0)
                xpos=body_w/2+1;
        }

        else
        {
            if (max_w()>W)
                xpos=W-body_w/2-1;
        }

    }

    bool movePlayer(player * Opponent, int scores[])
    {
        if (activity[5])
            return false;

        bool change=false;
        bool canPowerUp=powerBar(scores);

        //kick
        if (keyboard_state_array[this->controls.KICK] && !activity[3])
        {
            activity[3]=true;
            change =true;
        }
        else
            activity[3]=false;

        //go right
        if (keyboard_state_array[this->controls.RIGHT])
        {
            activity[2]=true;
            frameCount++;
            xpos+=r_speed;
            change= true;
        }

        //go left
        else if (keyboard_state_array[this->controls.LEFT])
        {
            activity[1]=true;
            frameCount++;
            xpos-=r_speed;
            change= true;
        }

        //jump
        if (keyboard_state_array[this->controls.JUMP] && !jumping)
        {
            jumping=true;
            change= true;

        }

        //power Up
        if(keyboard_state_array[this->controls.POW] && canPowerUp)
        {
            if (!activity[4])
            {
                if (specialPower=='h')
                    fireballSplashScreen();
                if (specialPower=='h' || specialPower=='k')
                {
                    Mix_Music*fbMusic=Mix_LoadMUS("./Music/fb.mp3");
                    Mix_PlayMusic(fbMusic,-1);
                }

                setPowerUpStartTime();
                activity[4]=true;
            }

            change =true;
        }

        if (!change)
        {
            if (activity[4])
            {
                clearActivity();
                activity[4]=true;
            }
            else
                clearActivity();

            frameCount=0;
        }

        //check for collisions
        else
        {
            if (!activity[1]&&!activity[2] && !activity[4])
                activity[0]=false;
            intersectsWith(Opponent);
            isOutOfBounds();
        }

        return change;

    }

    void drawKickRange()
    {
        int num=80;
        int numy=80;

        Sint16 vx[4]= {xpos,xpos+num*scaling_factor,xpos+num*scaling_factor,xpos};
        Sint16 vy[4]= {ypos+numy*scaling_factor,ypos+numy*scaling_factor,max_h()-17*scaling_factor,max_h()-17*scaling_factor};
        filledPolygonColor(m_renderer,vx,vy,4,0x9fffffff);

        Sint16 Vx[4]= {xpos+num*scaling_factor,xpos+2*num*scaling_factor,xpos+2*num*scaling_factor,xpos+num*scaling_factor};
        Sint16 Vy[4]= {max_h()-50*scaling_factor,max_h()-50*scaling_factor,max_h()+20*scaling_factor,max_h()+20*scaling_factor};
        filledPolygonColor(m_renderer,Vx,Vy,4,0x9fffff00);

        SDL_RenderPresent(m_renderer);

        SDL_Event* eve=new SDL_Event();
        while (eve->type!=SDL_KEYDOWN)
            SDL_PollEvent(eve);
    }

    //0 for outside of kick range. 1 for top of the shoe. 2 for middle. 3 for bottom
    int kickRange(int x, int y)
    {
        if (!activity[3])
            return 0;

        int num=100;
        int numy=80;

//        drawKickRange();

        if (player_num==1)
        {
            if (x>=xpos && x<=xpos+num*scaling_factor && y<=max_h()-17*scaling_factor && y>=ypos+numy*scaling_factor)
                return 1;

            else if (x>=xpos+num*scaling_factor && x<=xpos+1.2*num*scaling_factor
                     && y<=max_h()+20*scaling_factor && y>=max_h()-50*scaling_factor)
                return 2;

            else if (x>=xpos && x<=xpos+num*scaling_factor && y<=max_h()+34*scaling_factor && y>=max_h()-17*scaling_factor)
                return 3;

            else
                return 0;
        }
        else
        {
            if (x<=xpos && x>=xpos-num*scaling_factor && y<=max_h()-17*scaling_factor && y>=ypos+numy*scaling_factor)
                return 1;

            else if (x<=xpos-num*scaling_factor && x>=xpos-1.2*num*scaling_factor
                     && y<=max_h()+20*scaling_factor && y>=max_h()-50*scaling_factor)
                return 2;

            else if (x<=xpos && x>=xpos-num*scaling_factor && y<=max_h()+34*scaling_factor && y>=max_h()-17*scaling_factor)
                return 3;

            else
                return 0;
        }


    }


    void drawPlayer()
    {
        SDL_Texture * skin;
        SDL_Rect skin_rect;
        SDL_Texture * jersey;
        SDL_Rect rect;

        loadHeadTexture(&skin);
        loadJerseyTexture(&jersey);

        skin_rect.x= xpos-head_r;
        skin_rect.y= ypos-head_r;
        skin_rect.w=2*head_r;
        skin_rect.h=2*head_r;
        rect.y= ypos+head_r;
        if (!activity[3] || activity[1] || activity[2])
        {
            rect.x= xpos-body_w/2;
            rect.w=body_w;
        }
        else
        {
            rect.w=body_w+24*scaling_factor;
            rect.x= xpos-rect.w/2;
            if (player_num==1)
                skin_rect.x+=10*scaling_factor;
            else
                skin_rect.x-=10*scaling_factor;
        }
        rect.h=body_h;

        //animate texture
        SDL_Rect srect;
        srect.y=0;
        srect.w=56;
        srect.h=120;

        if (activity[0])
            srect.x=0;

        if (jumping)
        {
            //moving up
            if (j_speed>0)
                srect.x=3*56;
            else
                srect.x=0;

            ypos-=j_speed;
            j_speed-=gravity;

            if (ypos>groundheight)
            {
                activity[0]=true;
                endJump();
            }
        }

        else if( activity[1] || activity[2])
        {
            int realFrame=frameCount/5;
            if (player_num==1)
                skin_rect.x+=10*(realFrame%2);
            else
                skin_rect.x-=10*(realFrame%2);

            skin_rect.y-=5*(realFrame%2);
            rect.y-=5*(realFrame%2);
            srect.x=56*(1+realFrame%2);
        }

        else if (activity[3])
        {
            srect.w+=28;
            srect.x=224;
        }



        if (player_num==1)
        {
            SDL_RenderCopy(m_renderer,jersey,&srect,&rect);
            SDL_RenderCopy(m_renderer,skin,NULL,&skin_rect);
        }
        else
        {
            SDL_RenderCopyEx(m_renderer,jersey,&srect,&rect,0,NULL,SDL_FLIP_HORIZONTAL);
            SDL_RenderCopyEx(m_renderer,skin,NULL,&skin_rect,0,NULL,SDL_FLIP_HORIZONTAL);
        }

        SDL_DestroyTexture(skin);
        SDL_DestroyTexture(jersey);
    }


//    void clone(player *newPlayer)
//    {
//        newPlayer->skin_rect=this->skin_rect;
//        newPlayer->skin=this->skin;
//        newPlayer->rect=this->rect;
//        newPlayer->jersey=this->jersey;
//        newPlayer->inGame=true;
//        newPlayer->xpos=this->xpos;
//        newPlayer->ypos=this->ypos;
//        for (int i=0;i<5;i++)
//            newPlayer->activity[i]=this->activity[i];
//        newPlayer->player_num=this->player_num;
//    }
    void ateStar()
    {
        starBoost++;
    }

    void ateBomb()
    {
        body_w*=1.2;
        body_h*=1.2;
        ypos-=0.2*body_h;
        groundheight-=0.2*body_h;
    }
};

class image
{
private:
    SDL_Renderer *ren;
    SDL_Rect img_rect;
public:
    string path;
    SDL_Texture *img;
    int img_w, img_h;
    int x;
    int y;
    float sc_x;
    float sc_y;
    void run()
    {
        img = IMG_LoadTexture(ren, path.c_str());
        SDL_QueryTexture(img, NULL, NULL, &img_w, &img_h);
    }

    void load()
    {
        //const char* cpath = path.c_str();


        img_rect.x = x;
        img_rect.y = y;
        img_rect.w = img_w * sc_x;
        img_rect.h = img_h * sc_y;
        SDL_RenderCopy(ren, img, NULL, &img_rect);
        //delete cpath;
    }
    image(string Path, int X, int Y, float SC, SDL_Renderer *Ren)
    {
        x = X;
        y = Y;
        path = Path;
        sc_x = SC;
        sc_y = SC;
        ren = Ren;
        run();
        load();
    }
    void setCenter(int X, int Y)
    {
        x = X - img_w * sc_x / 2;
        y = Y - img_h * sc_y / 2;
        load();
    }
    void render()
    {
        SDL_RenderCopy(ren, img, NULL, &img_rect);
    }
    ~image()
    {
        SDL_DestroyTexture(img);
    }
};
class btn
{
private:
    int x;
    int y;
    int he;
    int wi;
public:
    image *pic;
    btn(int X, int Y, string Path, float SC,SDL_Renderer *Ren)
    {
        pic = new image(Path, X, Y, SC, Ren);
        x = X;
        y = Y;
        he = pic->img_h;
        wi = pic->img_w;
    }
    ~btn()
    {
        delete pic;
    }
    void setXY(int X, int Y)
    {
        x = X;
        y = Y;
        pic->x = X;
        pic->y = Y;
        pic->load();
    }
    void setCenter(int X, int Y)
    {
        x = X - wi * pic->sc_x / 2;
        y = Y - he * pic->sc_y / 2;
        pic->x = x;
        pic->y = y;
        pic->load();
    }
    bool btn_clicked(int mouse_x, int mouse_y)
    {
        if(mouse_x >= x && mouse_x <= x + wi * pic->sc_x
                && mouse_y >= y && mouse_y <= y + he * pic->sc_y)
            return true;
        else
            return false;
    }

};
struct plyr
{
    string name;
    int score;
    int goals;
    int wins;
    int losts;
    int played;
};

class playerInfo
{
private:
    SDL_Renderer *ren;
    SDL_Texture *mTexture;
    SDL_Texture *mmTexture;
    SDL_Rect img_rect;
    //file.open("db.txt", ios::out | ios::in );
    vector<plyr> plist;
    int exists(string name)
    {
        for(int i = 0; i < plist.size(); i++)
        {
            if(plist.at(i).name == name)
                return i;
        }
        return -1;
    }
    string split(int st, int fi, string str)
    {
        string ret = "";
        for(; st <= fi; st++)
            ret += str[st];
        return ret;
    }
    plyr infosplit(string str)
    {
        struct plyr pl;
        int st = 0;
        string strlist[5];
        int counter = 0;
        for(int i = 0; i < str.length(); i++)
        {
            if(str[i] == ',')
            {
                strlist[counter++] = split(st, i - 1, str);
                st = i + 1;
            }
        }
        for(int i = 0; i < 5; i++)
            cout<<strlist[i]<<endl;
        pl.name = strlist[0];
        pl.score = stoi(strlist[1]);
        pl.goals = stoi(strlist[2]);
        pl.wins = stoi(strlist[3]);
        pl.losts = stoi(strlist[4]);
        pl.played = stoi(split(st, str.length() - 1, str));
        return pl;
    }
public:
    struct plyr p1;
    struct plyr p2;
    playerInfo()
    {
        ifstream file("db.txt");
        string tmp;
        while(getline(file, tmp))
        {
            if(file.good())
            {
                struct plyr pl = infosplit(tmp);
                plist.push_back(pl);
            }
        }
        file.close();
    }
    void addPlayer1(string name)
    {
        int id = exists(name);
        if(id == -1)
        {
            p1.name = name;
            p1.score = -1;
            p1.goals = 0;
            p1.wins = 0;
            p1.losts = 0;
            p1.played = 0;
            plist.push_back(p1);
        }
        else
        {
            p1 = plist[id];
        }
    }

    void addPlayer2(string name)
    {
        int id = exists(name);

        if(id == -1)
        {
            p2.name = name;
            p2.score = -1;
            p2.goals = 0;
            p2.wins = 0;
            p2.losts = 0;
            p2.played = 0;
            plist.push_back(p2);
        }
        else
        {
            p2 = plist[id];
        }
    }
    void update(string name, int score)
    {
        int id = exists(name);
        if(id != -1)
        {
            plist[id].score = score;
        }
    }
    void del(string name)
    {
        int id = exists(name);
        if(id != -1)
        {
            plist.erase(plist.begin() + id);
        }
    }

    int leaderboardItem(SDL_Renderer *Ren)
    {
        sortlist();
        ren = Ren;
        mTexture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, 490, (plist.size() - 2) * 50);
        SDL_SetRenderTarget(ren, mTexture);
        SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
        SDL_RenderClear(ren);
        int x = 20,y = 0, he = 40;

        int j = 0;
        for(int i = plist.size() - 1; i >= 0 ; i--)
        {
            if(plist[i].score != -1)
            {
                textRGBA(ren, x, y, plist[i].name.c_str(), 1, 45, 0, 0, 0, 255);
                textRGBA(ren, x + 350, y, to_string(plist[i].score).c_str(), 1, 45, 0, 0, 0, 255);
                y += he;
            }
        }
        SDL_SetRenderTarget(ren, NULL);
        return y - he * 2;
    }
    int leaderboardItemex(SDL_Renderer *Ren)
    {
        sortlist();
        ren = Ren;
        mmTexture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, 1080, (plist.size() - 2) * 70);
        SDL_SetRenderTarget(ren, mmTexture);
        SDL_SetTextureBlendMode(mmTexture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
        SDL_RenderClear(ren);
        int x = 80,y = 0, he = 70;

        int j = 0;
        for(int i = plist.size() - 1; i >= 0 ; i--)
        {
            if(plist[i].score != -1)
            {
                textRGBA(ren, x, y, plist[i].name.c_str(), 1, 30, 0, 0, 0, 255);
                textRGBA(ren, x + 160, y, to_string(plist[i].score).c_str(), 1, 30, 0, 0, 0, 255);
                textRGBA(ren, x + 320, y, to_string(plist[i].goals).c_str(), 1, 30, 0, 0, 0, 255);
                textRGBA(ren, x + 480, y, to_string(plist[i].wins).c_str(), 1, 30, 0, 0, 0, 255);
                textRGBA(ren, x + 720, y, to_string(plist[i].losts).c_str(), 1, 30, 0, 0, 0, 255);
                textRGBA(ren, x + 980, y, to_string(plist[i].played).c_str(), 1, 30, 0, 0, 0, 255);
                y += he;
            }
        }
        SDL_SetRenderTarget(ren, NULL);
        return y - he * 2;
    }
    void leaderboardItemRenderex(int X, int Y, int st_y)
    {
        img_rect.w = 1080;
        img_rect.h = 400;
        img_rect.x = X;
        img_rect.y = Y;
        SDL_Rect part;
        part.x = 0;
        part.y = st_y;
        part.w = 1080;
        part.h = 400;
        SDL_RenderCopy(ren, mmTexture, &part, &img_rect);
    }
    void leaderboardItemRender(int X, int Y, int st_y)
    {
        img_rect.w = 490;
        img_rect.h = 220;
        img_rect.x = X;
        img_rect.y = Y;
        SDL_Rect part;
        part.x = 0;
        part.y = st_y;
        part.w = 490;
        part.h = 220;
        SDL_RenderCopy(ren, mTexture, &part, &img_rect);
    }
    void save_list()
    {
        sortlist();
        ofstream file("db.txt", std::ios::out | std::ios::trunc);
        for(int i = 0; i < plist.size(); i++)
            file<<plist[i].name<<","<<plist[i].score<<","<<plist[i].goals<<","<<plist[i].wins<<","<<plist[i].losts<<","<<plist[i].played<<endl;
    }
    // :( sort based on score
    void sortlist()
    {
        for(int i = 0; i < plist.size(); i++)
        {
            for(int j = i + 1; j < plist.size(); j++)
            {
                if(plist[i].score > plist[j].score)
                    swap(plist[i], plist[j]);
            }
        }
    }
};

class ball
{
private:

    SDL_Point center;

    //0 for no fireball, 1 for player one, 2 for player two
    int fireball=0;

    int omega=10;

    double angle;

    //position of center
    int startpos=300;

    //velocity
    double xvel=0;
    double yvel=0;

    int defaultKickSpeed=81;
    int kickSpeed=81;
    int default_skin;

    int skn_code;

    //acceleration
    int gravity=5;
    double friction=1.5;

    int yground=670;

    void loadSkin(SDL_Texture ** skin)
    {
        string skn="./Textures/Ball/";
        skn.push_back('0'+skn_code);
        skn+=".png";
        *skin=IMG_LoadTexture(m_renderer,skn.c_str());
    }

    //distance between ball and a given point
    double dist (int x, int y)
    {
        return sqrt((xpos-x)*(xpos-x)+(ypos-y)*(ypos-y));
    }


//    struct effectChunks chunks;

public:

    int xpos=W/2;
    int ypos=startpos;
    int radius=40*scaling_factor;

    ball();
    ~ball()
    {}

    ball(int code,player * p_list)
    {
        skn_code=code;
        default_skin=code;
        angle=0;
        draw(p_list);
    }

    void lowerGravity()
    {
        gravity/=2;
    }

    void setXvel(int v)
    {
        xvel=v;
    }

    void setYvel(int v)
    {
        yvel=v;
    }

    bool isFireball()
    {
        if (fireball!=0)
            return true;

        return false;
    }

//    void freeChunks()
//    {
//        if (chunks.freebounceChunk)
//            Mix_FreeChunk(*chunks.bounce);
//
//        if (chunks.freeKickChunk)
//            Mix_FreeChunk(*chunks.kick);
//    }

    void Rotate()
    {
        if (velocity()>=4)
            angle+=velocity()*omega*0.3;

        if (angle>=360)
            angle-=360;
    }

    void setFireball (int num)
    {
        fireball=num;
    }

    void invisible(bool toggle)
    {
        if (toggle)
        {
            skn_code=0;
        }
        else
            skn_code=default_skin;
    }

    void drawGoals()
    {
        SDL_Texture * goalPost;
        goalPost=IMG_LoadTexture(m_renderer,"./Textures/g.png");

        SDL_Rect g_rect;
        g_rect.w=100*scaling_factor;
        g_rect.h=360*scaling_factor;
        g_rect.y=H-360*scaling_factor;

        g_rect.x=0;
        SDL_RenderCopy(m_renderer,goalPost,NULL,&g_rect);

        g_rect.x=W-g_rect.w;
        SDL_RenderCopyEx(m_renderer,goalPost,NULL,&g_rect,0,NULL,SDL_FLIP_HORIZONTAL);
        SDL_DestroyTexture(goalPost);

    }

    int fireCNT=0;

    void drawFire()
    {
        SDL_Texture* fireSkn;
        SDL_Rect f_rect;

        f_rect.h=2.7*radius;
        f_rect.w=2.7*radius;
        f_rect.x=xpos-2.7*radius/2;
        f_rect.y=ypos-2.7*radius/2;

        string address="./Textures/Ball/Fireball/";
        address+=to_string(fireCNT%5);
        address+=".png";

        fireSkn=IMG_LoadTexture(m_renderer,address.c_str());

        SDL_RenderCopy(m_renderer,fireSkn,NULL,&f_rect);
        SDL_DestroyTexture(fireSkn);
        fireCNT++;
    }

    void draw(player *p_list)
    {
        SDL_Texture * skin;
        loadSkin(&skin);


        Rotate();
        for (int i=0; i<6; i++)
        {
            if (p_list[i].inGame)
            {
                kickBall(&p_list[i]);
            }
        }
        changePos();
        for (int i=0; i<6; i++)
        {
            if (p_list[i].inGame)
            {
                reflect(&p_list[i]);
            }
        }
        squeezeOut(&p_list[0],&p_list[1]);


        SDL_Rect skn_rect;
        skn_rect.h=2*radius;
        skn_rect.w=2*radius;
        skn_rect.x=xpos-radius;
        skn_rect.y=ypos-radius;

        center.x=radius;
        center.y=radius;

        SDL_RenderCopyEx(m_renderer,skin,NULL,&skn_rect,angle,&center, SDL_FLIP_NONE);

        if (fireball!=0)
            drawFire();
        else
            fireCNT=0;

        SDL_DestroyTexture(skin);
        drawGoals();
    }

    //returns 0 for no goal,-1 for out, 1 for player 1, 2 for player 2
    int isGoal()
    {
        if (xpos-radius<100*scaling_factor)
        {
            if (ypos+radius<=360*scaling_factor)
                return -1;

            return 2;
        }
        else if (xpos+radius>W-100*scaling_factor)
        {
            if (ypos+radius<=360*scaling_factor)
                return -1;

            return 1;
        }
        else
            return 0;
    }

    double velocity()
    {
        return sqrt(xvel*xvel+yvel*yvel);
    }

    void slowDown()
    {
        if (velocity()!=0)
        {
            xvel-=xvel*friction/velocity();
            yvel-=yvel*friction/velocity();
        }
    }

    //movement
    void changePos()
    {
        xpos+=xvel;
        ypos+=yvel;

        bounce();
        yvel+=gravity;
        slowDown();

    }

    //bounce off of ground
    void bounce ()
    {
        if (ypos+radius>=H && yvel>0)
        {
            //change threshold if necessary
            if (yvel<=5)
                yvel=0;
            else
                yvel=-yvel;

            ypos=H-radius;

            //maybe add particle engine(grass)
        }
    }

    //reflect from player
    void reflect (player * m_player)
    {
        double d=dist(m_player->xpos,m_player->ypos);

        //reflect from body
        if (this->ypos <= m_player->ypos+m_player->head_r+m_player->body_h
                && this->ypos +radius >= m_player->ypos+m_player->head_r)
        {
            int vrel=m_player->getXVel()-xvel;
            if(this->xpos-radius<=m_player->xpos+m_player->body_w/2 && this->xpos-radius>m_player->xpos-m_player->body_w/2-vrel && vrel>=0)
            {
                Mix_PlayChannel(-1,bounceEffect,0);

                xvel=-xvel+2*m_player->getXVel();
                this->xpos=m_player->xpos+m_player->body_w/2+radius+1;

                if (fireball==m_player->getPlayerNum())
                {
                    if (vrel>0)
                    {
                        m_player->stun();
                        xvel/=100;
                        m_player->xpos-=200*scaling_factor;
                    }
                }
//
//                chunks.freebounceChunk=true;
//                chunks.bounce=&bounceEffect;

            }
            else if (this->xpos+radius>=m_player->xpos-m_player->body_w/2 && this->xpos+radius < m_player->xpos+m_player->body_w/2-vrel && vrel<=0)
            {
                Mix_PlayChannel(-1,bounceEffect,0);

                xvel=-xvel+2*m_player->getXVel();
                this->xpos=m_player->xpos-m_player->body_w/2-radius-1;

                if (fireball==m_player->getPlayerNum())
                {
                    if (vrel<0)
                    {
                        m_player->stun();
                        xvel/=100;
                        m_player->xpos+=200 * scaling_factor;
                    }
                }
//                chunks.freebounceChunk=true;
//                chunks.bounce=&bounceEffect;
            }
        }

        //reflect from head
        else if (d<=m_player->head_r+radius)
        {
            Mix_PlayChannel(-1,bounceEffect,0);

            int VXrel=xvel-m_player->getXVel();
            int VYrel=yvel-m_player->getYVel();
            int x_rel=this->xpos-m_player->xpos;
            int y_rel=this->ypos-m_player->ypos;

            //add elasticity coefficient if necessary
            xvel=xvel-2*(VXrel*x_rel+VYrel*y_rel)*x_rel/(d*d);
            yvel=yvel-2*(VXrel*x_rel+VYrel*y_rel)*y_rel/(d*d);

            int newX,newY;
            newX=m_player->xpos+x_rel*(radius+m_player->head_r+2)/d;
            newY=m_player->ypos+y_rel*(radius+m_player->head_r+2)/d;
            this->ypos=newY;
            this->xpos=newX;

//            chunks.freebounceChunk=true;
//            chunks.bounce=&bounceEffect;
        }

        //reflect from foot while jumping
        else if (m_player->isJumping() && this->xpos <= m_player->xpos+m_player->body_w/2
                 && this->xpos >= m_player->xpos-m_player->body_w/2 && this->ypos-radius <= m_player->ypos + m_player->head_r +m_player->body_h)
        {
            Mix_PlayChannel(-1,bounceEffect,0);

            yvel=-yvel+2*m_player->getYVel();
            this->ypos=m_player->ypos+m_player->head_r+m_player->body_h+radius+1;

//            chunks.freebounceChunk=true;
//            chunks.bounce=&bounceEffect;
        }

//        else
//        {
//            chunks.freebounceChunk=false;
//        }
    }

    int getKickSpeed ()
    {
        return (kickSpeed);
    }

    void kickBall(player * m_player)
    {
        int state=m_player->kickRange(xpos,ypos);

        if (state==0)
        {
//            chunks.freeKickChunk=false;
            return;
        }

        Mix_PlayChannel(-1,kickEffect,0);

        if (fireball!=m_player->getPlayerNum()&& fireball!=0)
        {
            state=2;
            kickSpeed*=1.5;
        }
        else
            kickSpeed=defaultKickSpeed;

        if (state==1)
        {
            yvel=kickSpeed*sin(50*M_PI/180);
            xvel=kickSpeed*cos(50*M_PI/180);
            if (m_player->xpos > xpos)
                xvel=-xvel;
        }
        else if (state==2)
        {
            yvel=kickSpeed*sin(15*M_PI/180);
            xvel=kickSpeed*cos(15*M_PI/180);
            if (m_player->xpos > xpos)
                xvel=-xvel;
        }
        else
        {
            yvel=-kickSpeed*sin(45*M_PI/180);
            xvel=kickSpeed*cos(45*M_PI/180);
            if (m_player->xpos > xpos)
                xvel=-xvel;
        }

//        chunks.freeKickChunk=true;
//        chunks.kick=&kickEffect;
    }

    bool isStuck(player * p_1, player * p_2)
    {
        if (this->ypos >= max(p_1->min_h(),p_2->min_h()) && this->ypos <= min(p_1->max_h(),p_2->max_h())
                && max(p_1->min_w(),p_2->min_w())-min(p_1->max_w(),p_2->max_w())<=2*radius)
        {
            if (this->xpos<=max(p_1->min_w(),p_2->min_w()) && this->xpos>=min(p_1->max_w(),p_2->max_w()))
            {
                return true;
            }
        }

        return false;
    }

    //if ball gets stuck between players
    void squeezeOut (player * p_1, player * p_2)
    {
        if (isStuck(p_1,p_2))
        {
            xvel=0;
            yvel=0;
//            if (p_1->max_w()<p_2->min_w())
//            {
//                int delta=2*radius+2-p_2->min_w()+p_1->max_w();
//                delta/=2;
//                p_2->xpos+=delta;
//                p_1->xpos-=delta;
//            }
//
//            else
//            {
//                int delta=2*radius+2-p_1->min_w()+p_2->max_w();
//                delta/=2;
//                p_1->xpos+=delta;
//                p_2->xpos-=delta;
//            }
////            p_2->xpos-=p_2->getXVel();
////            p_1->xpos-=p_1->getXVel();

            ypos=startpos;

        }
    }
};

class brick
{
private:
    int lives;
    bool inGame = true;

public:
    SDL_Rect b_rect{0, 0, 100, 50};
    brick()
    {
    }
    brick(int x, int y, int health)
    {
        b_rect.x = x;
        b_rect.y = y;
        this->lives = health;
        render();
    }
    void render()
    {
        if (!inGame)
            return;
        roundedBoxRGBA(m_renderer, b_rect.x - 3, b_rect.y - 3, b_rect.x + b_rect.w + 3, b_rect.y + b_rect.h + 3, b_rect.h / 4, 255 * lives / 4, 40, 110, 255);
        roundedBoxRGBA(m_renderer, b_rect.x, b_rect.y, b_rect.x + b_rect.w, b_rect.y + b_rect.h, b_rect.h / 4, 255 * lives / 4, 50, 170, 255);
    }
    void loseHealth()
    {
        lives--;
        if (lives == -1)
            inGame = false;
    }
    bool isInGame()
    {
        return inGame;
    }
};
class minigame
{
private:
    SDL_Texture *background;

public:
    int imax, jmax;
    brick **gameBricks;
    SDL_Rect rectSlider;
    minigame(int Imax, int Jmax)
    {
        imax = Imax;
        jmax = Jmax;
        rectSlider = {W / 2 - 50, H - 100 - 20, 100, 40};
        srand(time(NULL));
        gameBricks = new brick *[imax];
        for (int i = 0; i < imax; i++)
        {
            gameBricks[i] = new brick[jmax];
            for (int j = 0; j < jmax; j++)
                gameBricks[i][j] = brick(W / 2 - imax * 120 / 2 + i * 120, j * 60, rand() % 4);
        }
    }
    void showMiniGame()
    {
        for (int i = 0; i < imax; i++)
            for (int j = 0; j < jmax; j++)
                gameBricks[i][j].render();
        updateAndRenderSlider();
    }
    void updateAndRenderSlider()
    {
        SDL_PumpEvents();
        const Uint8 *keyboardArr = SDL_GetKeyboardState(NULL);
        rectSlider.x += 4 * (keyboardArr[SDL_SCANCODE_RIGHT] - keyboardArr[SDL_SCANCODE_LEFT]);
        roundedBoxRGBA(m_renderer, rectSlider.x - 10, rectSlider.y - 10, rectSlider.x + rectSlider.w + 10, rectSlider.y + rectSlider.h + 10, rectSlider.h / 2 + 10, 90, 40, 110, 255);
        roundedBoxRGBA(m_renderer, rectSlider.x, rectSlider.y, rectSlider.x + rectSlider.w, rectSlider.y + rectSlider.h, rectSlider.h / 2, 130, 50, 170, 255);
    }
    void reset()
    {
        for (int i = 0; i < imax; i++)
            for (int j = 0; j < jmax; j++)
                gameBricks[i][j] = brick(W / 2 - imax * 120 / 2 + i * 120, j * 60, rand() % 4);
    }
};
class mBall
{
private:
    // position of center
    int xpos = W / 2, ypos = 3 * H / 4;
    int default_velocity = 2;
    int xvel = default_velocity, yvel = -default_velocity;
    int radius = 40;

public:
    void render()
    {
        filledCircleColor(m_renderer, xpos, ypos, radius, 0xfffcf7de);
    }
    bool reflection() // reflect off of walls //returns false if player loses
    {
        if (ypos + radius >= H)
            return false;
        if (xpos + radius >= W || xpos - radius <= 0)
            xvel *= (-1);
        if (ypos - radius <= 0)
            yvel *= (-1);
        return true;
    }
    bool collision(minigame *m_game) // ingame collisions
    {
        xpos += xvel;
        ypos += yvel;
        if (!reflection())
            return false;
        SDL_Rect b_rect{xpos - radius, ypos - radius, 2 * radius, 2 * radius};
        // reflect from slider
        if (SDL_HasIntersection(&b_rect, &m_game->rectSlider) && xpos >= m_game->rectSlider.x && xpos <= m_game->rectSlider.x + m_game->rectSlider.w)
        {
            yvel = -yvel;
            ypos = m_game->rectSlider.y - radius - 2;
        }
        for (int i = 0; i < m_game->imax; i++)
            for (int j = 0; j < m_game->jmax; j++)
                if (m_game->gameBricks[i][j].isInGame())
                {
                    int state = checkCollisionState(&m_game->gameBricks[i][j]);
                    if (state != 0)
                        m_game->gameBricks[i][j].loseHealth();
                    if (state == 1)
                        xvel = -xvel;
                    else if (state == 2)
                        yvel = -yvel;
                }
        return true;
    }
    int checkCollisionState(brick *m_brick) // returns 0 if no collisions occur, 1 if x-col, 2 if y-col
    {
        // x-col
        if (ypos >= m_brick->b_rect.y && ypos <= m_brick->b_rect.y + m_brick->b_rect.h && xpos + radius >= m_brick->b_rect.x && xpos - radius <= m_brick->b_rect.x + m_brick->b_rect.w)
        {
            if (xvel<0)
                xpos=m_brick->b_rect.x + m_brick->b_rect.w+radius+1;
            else
                xpos=m_brick->b_rect.x-radius-1;
            return 1;
        }

        // y-col
        if (xpos >= m_brick->b_rect.x && xpos <= m_brick->b_rect.x + m_brick->b_rect.w && ypos + radius >= m_brick->b_rect.y && ypos - radius <= m_brick->b_rect.y + m_brick->b_rect.h)
        {
            if (yvel<0)
                ypos=m_brick->b_rect.y + m_brick->b_rect.h+radius+1;
            else
                ypos=m_brick->b_rect.y-radius-1;
            return 2;
        }

        return 0;
    }

    void reset()
    {
        xpos = W / 2, ypos = 3 * H / 4;
        xvel = default_velocity, yvel = -default_velocity;
    }
};

// function definition
float ease_bounceOut(float start, float final, int time, int total);
float ease_circ_in(int start_point, int end_point, int current_time, int total_time);
float easeBackOut(float p1, float p2, int time, int totalTime, float overshoot);
float easeBackIn(float p1, float p2, int time, int totalTime, float overshoot);
void init (SDL_Window * m_window);
int splashScreen(playerInfo *PI);
void update();
void punch (player* p_list, int index);
void invisibleBall (ball * B, player * P);
void scoreBoard(int scores[], int e_time);
void loadBG (int i);
void goalScored();
void headFireball (player * p_list, int index, ball * B);
void kickFireball(player * p_list, int index, ball * B);
void powerUp(player * p_list, ball * b);
void BallOut();
void unStun (player *p_list);
void clone(player* p_list, int index);
void resetPlayerList(player*p_list, ball *Ball);
bool newGame(int scores[], timer* time_keeper, int gameDuration, char codes[],SDL_Window*m_window);
int startMenu(playerInfo *PI);
void quit(SDL_Window * m_window);
int endMenu(playerInfo *PI, int score1, int score2);
void showText(int x, int y, int width, int height, string text, string fontName, int size, SDL_Color color, int alignVertical, int alignHorizontal, int angle);
setting select(playerInfo *PI);
setting newSelect(playerInfo *PI, int nhe);
SDL_Texture leaderboardItem(playerInfo PI);
int pauseMenu(SDL_Window*m_window);
void settingPage();
void surprise();
string textInput();


int main( int argc, char * argv[] )
{
    string str[2];
    for(int i = 1; i < argc; i++)
        str[i - 1] = argv[i];
    int nhe = 0;
    if(str[0] != "1")
        nhe++;
    if(str[1] != "1")
        nhe++;
    cout<<nhe<<endl;


    srand(time(NULL));
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048);
    srand(time(NULL));
    SDL_Window * m_window;
    init(m_window);
    Mix_Music *bgsound = Mix_LoadMUS("Music/mm.mp3");
    bgMusic=Mix_LoadMUS("./Music/bg2.mp3");
    kickEffect=Mix_LoadWAV("./Music/Kick.wav");
    bounceEffect=Mix_LoadWAV("./Music/Boink.wav");
    starSound=Mix_LoadWAV("./Music/coin.wav");
    Mix_VolumeMusic(77);
    ///game
    // get player information from the file
    playerInfo *PI = new playerInfo();
    int scores[2]= {0,0};
    while(stat != -1)
    {
        if(stat == 0)
        {
            Mix_PlayMusic(bgsound, -1);
            stat = splashScreen(PI);
        }
        if(stat == 1)
        {
            scores[0] = 0;
            scores[1] = 0;
            stat = startMenu(PI);
        }
        if(stat == 2)
        {
            struct setting pl1 = newSelect(PI, nhe);
            struct setting pl2 = newSelect(PI, nhe);
            Mix_HaltMusic();
            int time = (pl1.sec + pl2.sec) / 2;

            char codes[5]= {pl1.head+'0',pl2.head+'0',pl1.body+'0',pl2.body+'0','1'};

            if(rand() % 2 == 0)
            {
                loadBG(pl1.bg);
                codes[4]=pl2.ball+'0';
            }
            else
            {
                loadBG(pl2.bg);
                codes[4]=pl1.ball+'0';
            }


            if (pl1.head>=10)
                codes[0]='A'+pl1.head-10;

            if (pl2.head>=10)
                codes[1]='A'+pl2.head-10;

            //game clock
            timer timeKeeper;
            scores[0] = 0;
            scores[1] = 0;

            ///game
            timeKeeper.setGameStart();
            timeKeeper.pauseTime();
            while (newGame(scores, &timeKeeper, time, codes,m_window))
            {
                timeKeeper.setGameStart();
                timeKeeper.pauseTime();
            }
            Mix_PlayMusic(bgsound,-1);
            stat = 3;
        }
        if(stat == 3)
        {
            beginningOfNewGameSeries=true;
            stat = endMenu(PI, scores[0], scores[1]);
        }
    }

    Mix_FreeMusic(bgsound);
    quit(m_window);
    return 0;

}
void quit(SDL_Window * m_window)
{
    surprise();

    Mix_FreeChunk(kickEffect);
    Mix_FreeChunk(bounceEffect);
    Mix_FreeChunk(starSound);
    SDL_DestroyWindow( m_window );
    SDL_DestroyRenderer( m_renderer );
    SDL_DestroyTexture(background);
    IMG_Quit();
    SDL_Quit();
    exit(0);
}

void loadBG (int i)
{
    string str="./Textures/Background/";
    str.push_back('0'+i);
    str+=".jpg";
    background=IMG_LoadTexture(m_renderer,str.c_str());

    update();
}

void scoreBoard(int scores[], int e_time)
{
    SDL_Texture * scoreB;
    SDL_Rect s_rect;
    s_rect.h=128;
    s_rect.w=200;
    s_rect.x=W/2-100;
    s_rect.y=10;

    scoreB=IMG_LoadTexture(m_renderer,"./Textures/scoreboard.png");
    SDL_RenderCopy(m_renderer,scoreB,NULL,&s_rect);
    SDL_DestroyTexture(scoreB);

    string str=to_string(scores[0]);

    SDL_Color color;
    color.a=255;
    color.r=255;
    color.g=0;
    color.b=0;

    showText(W/2-59,103,0,0,str,"GothamRounded.ttf",35,color,0,1,0);
//    textRGBA(m_renderer,W/2-78,87,str.c_str(),2,35,255,0,0,255);
    str=to_string(scores[1]);
    showText(W/2+59,103,0,0,str,"GothamRounded.ttf",35,color,0,1,0);
//    textRGBA(m_renderer,W/2+38,87,str.c_str(),2,35,255,0,0,255);

    color.g=255;
    color.b=255;

    string time_str=to_string(e_time/60)+":"+to_string(e_time%60);

    showText(W/2,43,0,0,time_str,"GothamRounded.ttf",40,color,0,1,0);
}

//new text rgba
void textRGBA(SDL_Renderer*m_renderer, int x, int y, const char* text,int useless,int f_size,int r, int g, int b, int a)
{
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("./GothamRounded.ttf", f_size);
    int textWidth, textHeight;
    TTF_SizeText(font, text, &textWidth, &textHeight);

    SDL_Rect rectText{x, y, 0, 0};
    SDL_Color color{r,g,b,a};

    SDL_Surface *textSur = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *textTex = SDL_CreateTextureFromSurface(m_renderer, textSur);
    SDL_FreeSurface(textSur);
    SDL_QueryTexture(textTex, nullptr, nullptr, &rectText.w, &rectText.h);
    SDL_RenderCopy(m_renderer, textTex, nullptr, &rectText);
    SDL_DestroyTexture(textTex);
    TTF_CloseFont(font);
}

int pauseMenu(SDL_Window*m_window)
{
    int ret = -1;
    int fps = 1000 / 40;
    //background
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );
    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    // vars for anime
    int total = fps / 2, time = 0;
    int ic_x = W / 2 - 75, ic_st_y = -80, ic_fi_y = 50;
    int pl_x = W / 2, pl_y = 280, pl_st_y = H + 50, pl_fi_y = pl_y;
    int ho_x = W / 2, ho_y = 420, ho_st_y = H + 100, ho_fi_y = ho_y;
    int set_x = W / 2, set_y = 560, set_st_y = H + 150, set_fi_y = set_y;
    int ex_x = W / 2, ex_y = 700, ex_st_y = H + 200, ex_fi_y = ex_y;
    float ic_sc = 0.5;
    image *ic = new image("pic/icon.jpg", ic_x, ic_st_y, ic_sc, m_renderer);
    btn *pl_btn = new btn(pl_x, pl_st_y, "pic/startbtn.png", 0.5, m_renderer);
    pl_btn->setCenter(pl_x, pl_st_y);
    btn *ho_btn = new btn(ho_x, ho_st_y, "pic/endgame.png", 0.5, m_renderer);
    ho_btn->setCenter(ho_x, ho_st_y);
    btn *ex_btn = new btn(ex_x, ex_st_y, "pic/exitbtn.png", 0.5, m_renderer);
    ex_btn->setCenter(ex_x, ex_st_y);
    btn *set_btn = new btn(set_x, set_st_y, "pic/settingbtn.png", 0.5, m_renderer);
    set_btn->setCenter(set_x, set_st_y);
    SDL_Color cl;
    cl.r = 255;
    cl.g = 255;
    cl.b = 255;
    cl.a = 255;
    SDL_RenderPresent(m_renderer);
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );
    bg->render();
    while(time <= total)
    {
        ic->y = ease_bounceOut(ic_st_y, ic_fi_y, time, total);
        ic->load();
        pl_btn->setCenter(pl_x, easeBackOut(pl_st_y, pl_fi_y, time, total, 2.0));
        ho_btn->setCenter(ho_x, easeBackOut(ho_st_y, ho_fi_y, time, total, 2.0));
        ex_btn->setCenter(ex_x, easeBackOut(ex_st_y, ex_fi_y, time, total, 2.0));
        set_btn->setCenter(set_x, easeBackOut(set_st_y, set_fi_y, time, total, 2.0));
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
        time++;
    }
    SDL_Event *e = new SDL_Event();
    bool ren = true, plren = false, horen = false, exren = false, setren = false;
    int mx, my;
    while(true)
    {
        SDL_PollEvent(e);
        mx = e->motion.x;
        my = e->motion.y;
        // bover
        if(pl_btn->btn_clicked(mx, my))
        {
            pl_btn->pic->sc_x = .6;
            pl_btn->pic->sc_y = .6;
            pl_btn->setCenter(pl_x, easeBackOut(pl_st_y, pl_fi_y, time, total, 2.0));
            ren = true;
            plren = true;
        }
        else if(plren)
        {
            pl_btn->pic->sc_x = .5;
            pl_btn->pic->sc_y = .5;
            pl_btn->setCenter(pl_x, easeBackOut(pl_st_y, pl_fi_y, time, total, 2.0));
            plren = false;
            ren = true;
        }
        if(set_btn->btn_clicked(mx, my))
        {
            set_btn->pic->sc_x = .6;
            set_btn->pic->sc_y = .6;
            set_btn->setCenter(set_x, easeBackOut(set_st_y, set_fi_y, time, total, 2.0));
            ren = true;
            setren = true;
        }
        else if(setren)
        {
            set_btn->pic->sc_x = .5;
            set_btn->pic->sc_y = .5;
            set_btn->setCenter(set_x, easeBackOut(set_st_y, set_fi_y, time, total, 2.0));
            setren = false;
            ren = true;
        }
        if(ho_btn->btn_clicked(mx, my))
        {
            ho_btn->pic->sc_x = .6;
            ho_btn->pic->sc_y = .6;
            ho_btn->setCenter(ho_x, easeBackOut(ho_st_y, ho_fi_y, time, total, 2.0));
            ren = true;
            horen = true;
        }
        else if(horen)
        {
            ho_btn->pic->sc_x = .5;
            ho_btn->pic->sc_y = .5;
            ho_btn->setCenter(ho_x, easeBackOut(ho_st_y, ho_fi_y, time, total, 2.0));
            horen = false;
            ren = true;
        }
        if(ex_btn->btn_clicked(mx, my))
        {
            ex_btn->pic->sc_x = .6;
            ex_btn->pic->sc_y = .6;
            ex_btn->setCenter(ex_x, easeBackOut(ex_st_y, ex_fi_y, time, total, 2.0));
            ren = true;
            exren = true;
        }
        else if(exren)
        {
            ex_btn->pic->sc_x = .5;
            ex_btn->pic->sc_y = .5;
            ex_btn->setCenter(ex_x, easeBackOut(ex_st_y, ex_fi_y, time, total, 2.0));
            exren = false;
            ren = true;
        }

        if(e->type == SDL_MOUSEBUTTONDOWN && e->button.clicks == 1)
        {
            if(pl_btn->btn_clicked(mx, my))
            {
                ret = 2;
                break;
            }
            if(ho_btn->btn_clicked(mx, my))
            {
                ret = 1;
                break;
            }
            if(ex_btn->btn_clicked(mx, my))
            {
                quit(m_window);
                break;
            }
            if(set_btn->btn_clicked(mx, my))
            {
                settingPage();
            }
        }
        if(ren)
        {
            SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
            SDL_RenderClear( m_renderer );
            bg->render();
            ic->render();
            pl_btn->setCenter(pl_x, pl_y);
            ho_btn->setCenter(ho_x, ho_y);
            set_btn->setCenter(set_x, set_y);
            ex_btn->setCenter(ex_x, ex_y);
            SDL_RenderPresent(m_renderer);
            ren = false;
        }
        SDL_Delay(fps);
    }
    delete bg;
    delete ic;
    delete pl_btn;
    delete ho_btn;
    delete ex_btn;
    delete set_btn;
    delete e;
    return ret;
}

//p_num: which player has scored
void drawFans(int i, char ch1, char ch2,int p_num=0)
{
    SDL_Texture*fans;
    SDL_Rect f_rect;

    f_rect.w=150;
    f_rect.h=150;


    f_rect.x=120-f_rect.w/2;
    f_rect.y=200;

    string address="./Textures/Background/fansgif/";
    address.push_back(ch1);
    address+="/";
    address+=to_string(i%8);
    address+=".png";

    fans=IMG_LoadTexture(m_renderer,address.c_str());
    SDL_RenderCopy(m_renderer,fans,NULL,&f_rect);
    SDL_DestroyTexture(fans);

    SDL_Texture*fans2;
    address="./Textures/Background/fansgif/";
    address.push_back(ch2);
    address+="/";
    address+=to_string((i+4)%8);
    address+=".png";

//    if (p_num==2)
//    {
//        f_rect.w=360*(i)/36+150;
//        f_rect.h=f_rect.w;
//    }
//    else
//    {
//        f_rect.w=150;
//        f_rect.h=150;
//    }

    f_rect.x=W-120-f_rect.w/2;
    fans2=IMG_LoadTexture(m_renderer,address.c_str());
    SDL_RenderCopy(m_renderer,fans2,NULL,&f_rect);

    SDL_DestroyTexture(fans2);
}

void update()
{
    SDL_RenderCopy(m_renderer,background,NULL,NULL);
}

void clone(player* p_list, int index)
{
    p_list[index+2].inGame=true;
    p_list[index+4].inGame=true;
    p_list[index+2].ypos=p_list[index].ypos;
    p_list[index+4].ypos=p_list[index].ypos;
    p_list[index+2].xpos=index%2==0 ? p_list[index].xpos-120 : p_list[index].xpos+120;
    p_list[index+4].xpos=index%2==0 ? p_list[index].xpos-240 : p_list[index].xpos+240;

    //end powerUp
    if (p_list[index].powerUpDuration()>=8)
    {
        p_list[index].activity[4]=false;
        p_list[index].setLocalTime();
        p_list[index+2].inGame=false;
        p_list[index+4].inGame=false;
    }
}

void unStun (player *p_list)
{
    for (int i=0; i<2; i++)
    {
        if (p_list[i].activity[5] && p_list[i].stunDuration()>=3)
        {
            p_list->resetFrameCount();
            p_list[i].activity[5]=false;
        }
    }
}

void fireballSplashScreen ()
{
    SDL_Texture*hf;
    SDL_Rect hf_rect;
    hf_rect.h=500;
    hf_rect.w=700;
    hf_rect.x=W/2-350;
    hf_rect.y=H/2-250;

    hf=IMG_LoadTexture(m_renderer,"./Textures/hf.png");
    SDL_RenderCopy(m_renderer,hf,NULL,&hf_rect);
    SDL_RenderPresent(m_renderer);
    SDL_DestroyTexture(hf);
    SDL_Delay(400);
}

void headFireball (player * p_list, int index, ball * B)
{
    unStun(p_list);
    B->setFireball(2-index);

    if (B->ypos<=H-p_list[index].body_h)
    {
        B->ypos=150;
        if (index==0)
        {
            p_list[index].xpos=B->xpos-B->radius-p_list[index].body_w/2;
            p_list[index].ypos=B->ypos;

            double phi=atan((W-100*scaling_factor-B->xpos)/(H-p_list[index].ypos));
            double theta=atan((W-100*scaling_factor-B->xpos)/(H-p_list[index].ypos-360*scaling_factor));

            int k_speed=B->getKickSpeed();

            B->setXvel(k_speed*cos((phi+theta)/2));
            B->setYvel(k_speed*sin((phi+theta)/2));

        }
        else
        {
            p_list[index].xpos=B->xpos+B->radius+p_list[index].body_w/2;
            p_list[index].ypos=B->ypos;

            double phi=atan((-100*scaling_factor+B->xpos)/(H-p_list[index].ypos));
            double theta=atan((-100*scaling_factor+B->xpos)/(H-p_list[index].ypos-360*scaling_factor));

            int k_speed=B->getKickSpeed();

            B->setXvel(-k_speed*cos((phi+theta)/2));
            B->setYvel(k_speed*sin((phi+theta)/2));
        }

        p_list[index].hasJumped();

        Mix_PlayMusic(bgMusic,-1);
        B->setFireball(0);
        p_list[index].activity[4]=false;
        p_list[index].setLocalTime();
    }

    if (p_list[index].powerUpDuration()>=8)
    {
        Mix_PlayMusic(bgMusic,-1);

        B->setFireball(0);
        p_list[index].activity[4]=false;
        p_list[index].setLocalTime();
    }
}

void kickFireball(player * p_list, int index, ball * B)
{
    unStun(p_list);
    B->setFireball(2-index);

    if (p_list[index].powerUpDuration()>=8)
    {
        Mix_PlayMusic(bgMusic,-1);

        B->setFireball(0);
        p_list[index].activity[4]=false;
        p_list[index].setLocalTime();
    }

}

void punch (player* p_list, int index)
{
    if (!p_list[1-index].activity[5])
        p_list[1-index].stun();

    if (p_list[1-index].stunDuration()>=3)
    {
        p_list[index].activity[4]=false;
        p_list[index].setLocalTime();
        unStun(p_list);

    }
}

void invisibleBall (ball * B, player * P)
{
    B->invisible(true);

    if (P->powerUpDuration()>=3)
    {
        P->activity[4]=false;
        P->setLocalTime();
        B->invisible(false);
    }

}

void powerUp(player * p_list, ball * b)
{

    if (p_list[0].activity[4])
    {
        char sp=p_list[0].getSpecialPower();

        switch (sp)
        {
        case 'c':
            clone(p_list,0);
            break;

        case 'p':
            punch (p_list,0);
            break;

        case 'i':
            invisibleBall(b,&p_list[0]);
            break;

        case 'k':
            kickFireball(p_list,0, b);
            break;

        case 'h':
            headFireball(p_list,0,b);
            break;

        }
    }
    if (p_list[1].activity[4])
    {
        char sp=p_list[1].getSpecialPower();

        switch (sp)
        {
        case 'c':
            clone(p_list,1);
            break;

        case 'p':
            punch (p_list,1);
            break;

        case 'i':
            invisibleBall(b,&p_list[1]);
            break;

        case 'k':
            kickFireball(p_list,1, b);
            break;

        case 'h':
            headFireball(p_list,1,b);
            break;

        }
    }
}

void resetPlayerList(player*p_list, ball *Ball)
{
    delete [] p_list;
    cout<<"deleted p_list"<<endl;
//    delete (Ball);
//    cout<<"deleted Ball"<<endl;
}

void goalScored()
{
    Mix_Chunk*cheer=Mix_LoadWAV("./Music/cheer.wav");
    Mix_Chunk*whistle=Mix_LoadWAV("./Music/Whistle.wav");
    Mix_PlayChannel(-1,whistle,0);
    Mix_PlayChannel(-1,cheer,-1);


    SDL_Texture *g_texture;
    SDL_Rect g_rect;

    g_texture=IMG_LoadTexture(m_renderer,"./Textures/goal.png");

    for (int i=1; i<=10; i++)
    {
        g_rect.w=600*scaling_factor*0.1*i;
        g_rect.h=523*scaling_factor*0.1*i;
        g_rect.x=W/2-g_rect.w/2;
        g_rect.y=H/2-g_rect.h/2;
        SDL_RenderCopy(m_renderer,g_texture,NULL,&g_rect);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(50);
    }

    SDL_DestroyTexture(g_texture);
    SDL_Delay(3000);

    Mix_HaltChannel(-1);
    Mix_FreeChunk(cheer);
    Mix_FreeChunk(whistle);
}

void BallOut()
{
    Mix_Chunk*whistle=Mix_LoadWAV("./Music/Whistle.wav");
    Mix_PlayChannel(-1,whistle,-1);

    SDL_Texture *g_texture;
    SDL_Rect g_rect;

    g_texture=IMG_LoadTexture(m_renderer,"./Textures/out.png");


    for (int i=1; i<=10; i++)
    {
        g_rect.w=469*scaling_factor*0.1*i;
        g_rect.h=378*scaling_factor*0.1*i;
        g_rect.x=W/2-g_rect.w/2;
        g_rect.y=H/2-g_rect.h/2;
        SDL_RenderCopy(m_renderer,g_texture,NULL,&g_rect);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(50);
    }
    SDL_DestroyTexture(g_texture);
    SDL_Delay(500);

    Mix_HaltChannel(-1);
    Mix_FreeChunk(whistle);
}

//-1 for grow, 1 for shrink, 0 by default
void drawPortal(int x, int y, int i,int state=0)
{
    SDL_Texture*portal;
    SDL_Rect p_rect;
    p_rect.y=y;
    if (state==0)
    {
        p_rect.h=150;
        p_rect.w=500;
    }
    else if (state==1)
    {
        p_rect.h=(36-i)*150/36;
        p_rect.w=(36-i)*500/36;
    }
    else
    {
        p_rect.h=(i)*150/36;
        p_rect.w=(i)*500/36;
    }

    p_rect.x=x-p_rect.w/2;

    string address="./Textures/Background/portalgif/";
    address+=to_string(i%8);
    address+=".png";

    portal=IMG_LoadTexture(m_renderer,address.c_str());

    SDL_RenderCopy(m_renderer,portal,NULL,&p_rect);
    SDL_DestroyTexture(portal);
}

void newGameAnimation(player*p_list, int fps)
{
    Mix_Chunk*portal=Mix_LoadWAV("./Music/portal.wav");
    Mix_PlayChannel(-1,portal,0);

    for (int i=0; i<0.6*fps; i++)
    {
        drawPortal(p_list[0].xpos,20,i,-1);
        drawPortal(p_list[1].xpos,20,i,-1);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(1000/fps);
    }

    int gh=p_list[0].ypos;
    for (int i=0; i<=0.2*fps; i++)
    {
        update();
        drawPortal(p_list[0].xpos,20,i);
        drawPortal(p_list[1].xpos,20,i);
        p_list[0].ypos=ease_circ_in(100,gh, i*1000/fps, 200);
        p_list[1].ypos=p_list[0].ypos;
        p_list[0].drawPlayer();
        p_list[1].drawPlayer();
        SDL_RenderPresent(m_renderer);
        SDL_Delay(1000/fps);
    }

    for (int i=0; i<0.6*fps; i++)
    {
        update();
        p_list[0].drawPlayer();
        p_list[1].drawPlayer();
        drawPortal(p_list[0].xpos,20,i,1);
        drawPortal(p_list[1].xpos,20,i,1);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(1000/fps);
    }

    Mix_HaltChannel(-1);
    Mix_FreeChunk(portal);

    //music
    Mix_Music*bgMusic=Mix_LoadMUS("./Music/bg2.mp3");
    Mix_PlayMusic(bgMusic,-1);
}

void drawStars(star *s)
{
    if (s->isInGame())
    {
        int state=s->lifeSpan();

        if (state==0)
        {
            if (s->evenFrame())
            {
                s->render(true);
            }
            else
            {
                s->render();
            }
        }

        else if (state==1 && s->evenFrame())
        {
            s->render();
        }

        return;
    }

    if (rand()%70==0)
    {
        int y=500+rand()%300;
        int x=200+rand()%880;
        s->isBorn(x,y);
    }
}

void canEatStar (player* p, star* s)
{
    if (!s->isInGame())
        return;

    int s_xpos=s->getX(), s_ypos=s->getY();

    if (p->min_w()<=s_xpos && p->max_w()>=s_xpos && p->min_h()<=s_ypos && p->max_h()>=s_ypos)
    {
        Mix_PlayChannel(-1,starSound,0);
        p->ateStar();
        s->die();
    }
}

void drawBomb(bomb *b)
{
    if (b->isInGame())
    {
        int state=b->lifeSpan();

        if (state==0)
        {
            if (b->evenFrame())
            {
                b->render(true);
            }
            else
            {
                b->render();
            }
        }

        else if (state==1 && b->evenFrame())
        {
            b->render();
        }

        return;
    }

    if (rand()%100==0)
    {
        int y=500+rand()%300;
        int x=200+rand()%880;
        b->isBorn(x,y);
    }
}

void canEatBomb (player* p, bomb*b)
{
    if (!b->isInGame())
        return;

    int b_xpos=b->getX(), b_ypos=b->getY();

    if (p->min_w()<=b_xpos && p->max_w()>=b_xpos && p->min_h()<=b_ypos && p->max_h()>=b_ypos)
    {
        p->ateBomb();
        b->die();
    }
}

bool newGame(int scores[], timer* time_keeper, int gameDuration, char codes[],SDL_Window*m_window)
{
    //music
//    Mix_Music*bgMusic=Mix_LoadMUS("./Music/bg2.mp3");

    int fps=60;
    player *playerList=new player[6] {{codes[0],codes[2],1},{codes[1],codes[3],2},{codes[0],codes[2],1},{codes[1],codes[3],2},{codes[0],codes[2],1},{codes[1],codes[3],2}};

    if (beginningOfNewGameSeries)
        newGameAnimation(playerList,fps);

    beginningOfNewGameSeries=false;
    time_keeper->resumeTime();

    for (int i=2; i<6; i++)
        playerList[i].inGame=false;

    ball Ball (codes[4]-'0',playerList);
    star Star;
    bomb Bomb;

    SDL_RenderPresent(m_renderer);

    SDL_Event *e=new SDL_Event();
    e->type=NULL;
    int n=0;

//    Mix_PlayMusic(bgMusic,-1);

    while(1)
    {
        e->type=NULL;
        SDL_PollEvent(e);

        //cheat code
        if (e->key.keysym.sym==SDLK_t)
        {
            if (textInput()=="Sexy Face")
            {
                for (int i=0;i<6;i++)
                    playerList[i].lowerGravity();

                Ball.lowerGravity();
            }
        }

        if (e->key.keysym.sym==SDLK_ESCAPE)
        {
            time_keeper->pauseTime();
            stat= pauseMenu(m_window);

            if (stat==1)
            {
                beginningOfNewGameSeries=true;
                resetPlayerList(playerList, &Ball);
                return false;
            }

            time_keeper->resumeTime();
        }

        update();
        for (int i=0; i<6; i++)
            if (playerList[i].inGame)
                playerList[i].movePlayer(&playerList[1-i%2],scores);

        drawFans(n,codes[2],codes[3]);
        drawStars(&Star);
        drawBomb(&Bomb);
        canEatStar(&playerList[0],&Star);
        canEatStar(&playerList[1],&Star);
        canEatBomb(&playerList[1],&Bomb);
        canEatBomb(&playerList[0],&Bomb);
        powerUp(playerList, &Ball);

        for (int i=0; i<6; i++)
        {
            if (playerList[i].inGame)
            {
                playerList[i].drawPlayer();
            }
        }
        Ball.draw(playerList);

        scoreBoard(scores, time_keeper->getTotalElapsedTime());

        if (Ball.isGoal()!=0)
        {
            if (Ball.isFireball())
            {
                cout<<"fireball while goal or out"<<endl;
                Mix_PlayMusic(bgMusic,-1);
            }

            if (Ball.isGoal()==-1)
            {
                BallOut();
                return true;
            }

            scores[Ball.isGoal()-1]++;

            for (int j=0; j<6; j++)
            {
                if (playerList[j].inGame)
                {
                    playerList[j].drawPlayer();
                }
            }



            Ball.draw(playerList);
            scoreBoard(scores, time_keeper->getTotalElapsedTime());

            goalScored();
            resetPlayerList(playerList, &Ball);
            return true;
        }

        n++;
        n%=8;
        SDL_RenderPresent(m_renderer);
        SDL_Delay(1000/fps);

        if (time_keeper->getTotalElapsedTime()>=gameDuration)
        {
            beginningOfNewGameSeries=true;
            resetPlayerList(playerList,&Ball);
            return false;
        }
    }
}

string textInput()
{
    SDL_Color color{255,255,255,255};
    SDL_Delay(100);
    SDL_Texture*temp=SDL_CreateTexture(m_renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,1280,800);
    SDL_SetRenderTarget(m_renderer,temp);
    showText(W/2,200,0,0,"Enter cheat code!","./GothamRounded.ttf",50,color,0,1,0);
    SDL_RenderCopy(m_renderer,temp,NULL,NULL);
    SDL_SetRenderTarget(m_renderer,NULL);

    SDL_Event *e = new SDL_Event();
    int fps = 1000 / 15;
    string player1 = "";
    SDL_StartTextInput();
    int time = 1;
    while(true)
    {
        SDL_PollEvent(e);
        if(e->key.keysym.sym == SDLK_RETURN)
            break;
        if(time % 5 == 0)
        {
            time = 1;
            if(player1[player1.length() - 1] == '|')
            {
                player1.pop_back();
            }
            else
            {
                player1 += "|";
            }
        }
        if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_BACKSPACE && player1.length() > 0)
        {
            if(player1[player1.length() - 1] == '|')
            {
                player1.pop_back();
                player1.pop_back();
                player1 += '|';
            }
            else
            {
                player1.pop_back();
            }
            e->type = NULL;
        }
        if(e->type == SDL_TEXTINPUT)
        {
            if(player1[player1.length() - 1] == '|')
            {
                player1.pop_back();
                player1 += e->text.text;
                player1 += '|';
            }
            else
            {
                player1 += e->text.text;
            }
        }
        if(player1 != "")
            textRGBA(m_renderer, 500, 400, player1.c_str(), 1, 45, 255, 255, 255, 255);
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_RenderCopy(m_renderer,temp,NULL,NULL);
        time++;
    }

    SDL_StopTextInput();

    return player1;
}

void init (SDL_Window * m_window)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048);

    Uint32 SDL_flags = SDL_INIT_VIDEO | SDL_INIT_TIMER ;
    Uint32 WND_flags = SDL_WINDOW_SHOWN;

    SDL_Init( SDL_flags );
    SDL_CreateWindowAndRenderer( 1280, 800, WND_flags, &m_window, &m_renderer );
    //Pass the focus to the drawing window
    SDL_RaiseWindow(m_window);

    // Clear the window with a black background
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );

    SDL_RenderPresent( m_renderer );
}

int splashScreen(playerInfo *PI)
{
    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    SDL_Event *e = new SDL_Event();
    int fps = 1000 / 15;
    string player1 = "", player2 = "";
    // text input one
    SDL_StartTextInput();
    int time = 1;
    while(true)
    {
        SDL_PollEvent(e);
        if(e->key.keysym.sym == SDLK_RETURN)
            break;
        if(time % 5 == 0)
        {
            time = 1;
            if(player1[player1.length() - 1] == '|')
            {
                player1.pop_back();
            }
            else
            {
                player1 += "|";
            }
        }
        if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_BACKSPACE && player1.length() > 0)
        {
            if(player1[player1.length() - 1] == '|')
            {
                player1.pop_back();
                player1.pop_back();
                player1 += '|';
            }
            else
            {
                player1.pop_back();
            }
            e->type = NULL;
        }
        if(e->type == SDL_TEXTINPUT)
        {
            if(player1[player1.length() - 1] == '|')
            {
                player1.pop_back();
                player1 += e->text.text;
                player1 += '|';
            }
            else
            {
                player1 += e->text.text;
            }
        }
        if(player1 != "")
            textRGBA(m_renderer, 200, 600, player1.c_str(), 1, 45, 255, 255, 255, 255);
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
        time++;
    }
    if(player1[player1.length() - 1] == '|')
        player1.pop_back();
    time = 1;
    SDL_StartTextInput();
    e->type = 0;

    while(true)
    {
        SDL_PollEvent(e);
        if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_RETURN)
            break;
        if(time % 5 == 0)
        {
            time = 1;
            if(player2[player2.length() - 1] == '|')
            {
                player2.pop_back();
            }
            else
            {
                player2 += "|";
            }
        }
        if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_BACKSPACE && player2.length() > 0)
        {
            if(player2[player2.length() - 1] == '|')
            {
                player2.pop_back();
                player2.pop_back();
                player2 += '|';
            }
            else
            {
                player2.pop_back();
            }
        }
        if(e->type == SDL_TEXTINPUT)
        {
            if(player2[player2.length() - 1] == '|')
            {
                player2.pop_back();
                player2 += e->text.text;
                player2 += '|';
            }
            else
            {
                player2 += e->text.text;
            }
        }
        textRGBA(m_renderer, 200, 600, player1.c_str(), 1, 45, 255, 255, 255, 255);
        if(player2 != "")
            textRGBA(m_renderer, 880, 600, player2.c_str(), 1, 45, 255, 255, 255, 255);
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
        time++;
    }
    if(player2[player2.length() - 1] == '|')
        player2.pop_back();
    bool clicked = false;
    btn *letgo = new btn(W / 2 - 66, H / 2 - 66 - 100, "pic/letgo.png", 0.5, m_renderer);
    letgo->pic->render();
    textRGBA(m_renderer, 200, 600, player1.c_str(), 1, 45, 255, 255, 255, 255);
    textRGBA(m_renderer, 880, 600, player2.c_str(), 1, 45, 255, 255, 255, 255);
    SDL_RenderPresent( m_renderer );
    int mx, my;
    float dsc = -0.02, st_sc = 0.5, fi_sc = 1, sc = st_sc;
    bool anime = true;
    time = 0;
    int total_time = fps * 0.5;
    while(!clicked)
    {
        SDL_PollEvent(e);
        mx = e->motion.x;
        my = e->motion.y;
        if(time < total_time && anime)
        {
            sc = ease_bounceOut(st_sc, fi_sc, time, total_time);
            //cout<<sc<<endl;
            time ++;
        }
        if(time >= total_time || !anime)
        {
            anime = false;
            sc += dsc;
            sc = max((double)sc, 0.5);
            time --;
        }
        if(time <= 0)
        {
            time = 0;
            anime = true;
        }
        if(letgo->btn_clicked(mx, my) && e->type == SDL_MOUSEBUTTONDOWN)
            break;
        letgo->pic->sc_x = sc;
        letgo->pic->sc_y = sc;
        letgo->setXY(W / 2 - 135 * sc, H / 2 - 135 * sc - 100);
        textRGBA(m_renderer, 200, 600, player1.c_str(), 1, 45, 255, 255, 255, 255);
        textRGBA(m_renderer, 880, 600, player2.c_str(), 1, 45, 255, 255, 255, 255);
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps + 10);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
    }
    SDL_StopTextInput();
    PI->addPlayer1(player1);
    PI->addPlayer2(player2);
    delete bg;
    delete e;
    delete letgo;
    return 1;
}

void settingPage()
{
    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    image *popup = new image("pic/spopup.png", 100, 100, 0,m_renderer);
    popup->setCenter(W / 2, H / 2);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    SDL_Event *e = new SDL_Event();
    int fps = 1000 / 20;
    //anime vars
    int time = 0, total = fps / 4;
    float sc = 0;
    SDL_RenderPresent(m_renderer);
    // pop up opening anime
    while(time <= total)
    {
        sc = ease_bounceOut(0, 1.0, time, total);
        popup->sc_x = sc;
        popup->sc_y = sc;
        popup->setCenter(W / 2, H / 2);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        time++;
    }
    time = 0;
    total = fps / 3;
    int cl_x = W - 206, cl_y = 110, cl_st_x = W + 180, cl_fi_x = cl_x;
    int sl_y = -100, sl_st_y = sl_y, sl_en_y = H / 2;
    btn *close = new btn(cl_st_x, cl_y, "pic/close.png", 0.8, m_renderer);
    btn *slider = new btn(sl_x, sl_y, "pic/ball/1.png", 1.0, m_renderer);
    int yy = 0;
    while(time <= total)
    {
        yy = easeBackOut(-100, H / 2, time, total, 2.5);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        popup->render();
        boxRGBA(m_renderer, W / 2 - 300, yy - 10, W / 2 + 300, yy + 10, 255, 0, 0, 255);
        slider->setCenter(sl_x , easeBackOut(sl_st_y, sl_en_y, time, total, 2.5));
        //bar->setCenter(sl_x, easeBackOut(sl_st_y, sl_en_y, time, total, 2.5));
        close->setXY(easeBackOut(cl_st_x, cl_fi_x, time, total, 2.5), cl_y);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        time++;
    }
    bool ren = false, click = false;
    int mx, my;
    while(true)
    {
        SDL_PollEvent(e);
        if(e->type == SDL_MOUSEBUTTONDOWN)
            click = true;
        else if(e->type == SDL_MOUSEBUTTONUP)
            click = false;
        if(close->btn_clicked(e->motion.x, e->motion.y) && click)
            break;
        SDL_GetMouseState(&mx, &my);
        if(slider->btn_clicked(mx, my) && click)
        {
            if(mx <= W / 2 + 300 && mx >= W / 2 - 300)
            {
                slider->setCenter(mx, H / 2);
                sl_x = mx;
                Mix_VolumeMusic( int((double(mx - W / 2 + 300) / 600.0) * 128 * 0.6));
                Mix_Volume(-1,int((double(mx - W / 2 + 300) / 600.0) * 128 ));
                ren = true;
            }
        }
        if(ren)
        {
            SDL_RenderPresent(m_renderer);
            SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
            SDL_RenderClear(m_renderer);
            bg->render();
            popup->render();
            close->pic->render();
            boxRGBA(m_renderer, W / 2 - 300, yy - 10, W / 2 + 300, yy + 10, 255, 0, 0, 255);
            ren = false;
        }
        SDL_Delay(fps);
        e->type = 0;
    }
    time = 0;
    while(time <= total)
    {
        yy = easeBackOut(H / 2, -100, time, total,2.5);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        popup->render();
        close->setXY(easeBackIn(cl_fi_x, cl_st_x, time, total, 2.5), cl_y);
        boxRGBA(m_renderer, W / 2 - 300, yy - 10, W / 2 + 300, yy + 10, 255, 0, 0, 255);
        slider->setCenter(sl_x, easeBackIn(sl_x, sl_st_y, time, total, 2.5));
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        time++;
    }
    total = fps / 4;
    time = 0;
    while(time <= total)
    {
        sc = easeBackIn(1.0, 0, time, total, 2.0);
        popup->sc_x = sc;
        popup->sc_y = sc;
        popup->setCenter(W / 2, H / 2);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        time++;
    }
    time = 0;
    delete e;
    delete bg;
}

void aboutUs()
{

    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    image *popup = new image("pic/popup.png", 100, 100, 0,m_renderer);
    popup->setCenter(W / 2, H / 2);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    SDL_Event *e = new SDL_Event();
    int fps = 1000 / 20;
    //anime vars
    int time = 0, total = fps / 4;
    float sc = 0;
    SDL_RenderPresent(m_renderer);
    // pop up opening anime
    while(time <= total)
    {
        sc = ease_bounceOut(0, 1.0, time, total);
        popup->sc_x = sc;
        popup->sc_y = sc;
        popup->setCenter(W / 2, H / 2);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        time++;
    }
    time = 0;
    total = fps / 3;
    int cl_x = W - 206, cl_y = 110, cl_st_x = W + 180, cl_fi_x = cl_x;
    int ni_x = 150, ni_y = 220, ni_st_x = -200, ni_fi_x = ni_x;
    int ho_x = 150, ho_y = 440,ho_st_x = -200, ho_fi_x = ho_x;
    int hos_x = 370, hos_y = 530, hos_st_y = H + 100, hos_fi_y = hos_y;
    int nik_x = 370, nik_y = 310, nik_st_y = -100, nik_fi_y = nik_y;
    btn *close = new btn(cl_st_x, cl_y, "pic/close.png", 0.8, m_renderer);
    btn *nika = new btn(ni_fi_x, ni_y, "pic/nika.png", 1.0, m_renderer);
    btn *hosein = new btn(ho_fi_x, ho_y, "pic/hosein.png", 1.0, m_renderer);
    string niname = "Nika Zahedi", honame = "Hosein Anjidani", devtitle = "Developed By: ";
    int ttemp;

    while(time <= total)
    {
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        popup->render();
        ttemp = easeBackOut(-100, 125, time, total, 1.5);
        textRGBA(m_renderer, 180, ttemp, devtitle.c_str(), 1, 60, 0, 0, 0, 255);
        close->setXY(easeBackOut(cl_st_x, cl_fi_x, time, total, 2.5), cl_y);
        nika->setXY(easeBackOut(ni_st_x, ni_fi_x, time, total, 1.5), ni_y);
        hosein->setXY(easeBackOut(ho_st_x, ho_fi_x, time, total, 1.5), ho_y);
        textRGBA(m_renderer, nik_x, easeBackOut(nik_st_y, nik_fi_y, time, total, 1.5), niname.c_str(), 1, 45, 0, 0, 0, 255);
        textRGBA(m_renderer, hos_x, easeBackOut(hos_st_y, hos_fi_y, time, total, 1.5), honame.c_str(), 1, 45, 0, 0, 0, 255);
        showText(930, ttemp, 0, 0, "Special Thanks To : ", "fonts/BM.ttf", 40, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        ttemp = easeBackOut(W + 200, 930, time, total, 1.5);
        showText(ttemp, 185, 0, 0, "Dr. Arasteh", "fonts/BM.ttf", 45, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 255, 0, 0, "Mohammad Ali Sharifimehr", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 295, 0, 0, "Amir Reza Imani", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 335, 0, 0, "Mohammad Amin Eghlimi", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 375, 0, 0, "Sina Safizadeh", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 415, 0, 0, "Taher Abadi", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 455, 0, 0, "Moien Makkiyan", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 495, 0, 0, "Zeinab Ghazizade", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 535, 0, 0, "Mohammad Amin Eghlimi", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 575, 0, 0, "Ata Akbari", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 615, 0, 0, "Melika Rajabi", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 655, 0, 0, "Hossein AliHosseini", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        time++;
    }
    while(true)
    {
        SDL_PollEvent(e);
        if(e->type == SDL_MOUSEBUTTONDOWN)
        {
            if(close->btn_clicked(e->motion.x, e->motion.y))
                break;
        }
        SDL_Delay(fps);
        e->type = 0;
    }
    time = 0;
    while(time <= total)
    {
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        popup->render();
        ttemp = easeBackIn(125, -100, time, total, 1.5);
        textRGBA(m_renderer, 180, ttemp, devtitle.c_str(), 1, 60, 0, 0, 0, 255);
        close->setXY(easeBackIn(cl_fi_x, cl_st_x, time, total, 2.5), cl_y);
        nika->setXY(easeBackIn(ni_fi_x, ni_st_x, time, total, 1.5), ni_y);
        hosein->setXY(easeBackIn(ho_fi_x, ho_st_x, time, total, 1.5), ho_y);
        textRGBA(m_renderer, nik_x, easeBackIn(nik_fi_y, nik_st_y, time, total, 1.5), niname.c_str(), 1, 45, 0, 0, 0, 255);
        textRGBA(m_renderer, hos_x, easeBackIn(hos_fi_y, hos_st_y, time, total, 1.5), honame.c_str(), 1, 45, 0, 0, 0, 255);
        showText(930, ttemp, 0, 0, "Special Thanks To : ", "fonts/BM.ttf", 40, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        ttemp = easeBackIn(930, W + 200, time, total, 1.5);
        showText(ttemp, 185, 0, 0, "Dr. Arasteh", "fonts/BM.ttf", 45, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 255, 0, 0, "Mohammad Ali Sharifimehr", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 295, 0, 0, "Amir Reza Imani", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 335, 0, 0, "Mohammad Amin Eghlimi", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 375, 0, 0, "Sina Safizadeh", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 415, 0, 0, "Taher Abadi", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 455, 0, 0, "Moien Makkiyan", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 495, 0, 0, "Zeinab Ghazizade", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 535, 0, 0, "Mohammad Amin Eghlimi", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 575, 0, 0, "Ata Akbari", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 615, 0, 0, "Melika Rajabi", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        showText(ttemp, 655, 0, 0, "Hossein AliHosseini", "fonts/BM.ttf", 30, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        time++;
    }
    total = fps / 4;
    time = 0;
    while(time <= total)
    {
        sc = easeBackIn(1.0, 0, time, total, 2.0);
        popup->sc_x = sc;
        popup->sc_y = sc;
        popup->setCenter(W / 2, H / 2);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        time++;
    }
    time = 0;
}

void scorePopUp(playerInfo *PI)
{
    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    btn *popup = new btn(100, 100, "pic/spopup.png", 1.0, m_renderer);
    popup->setCenter(W / 2, H / 2);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    int tsize = PI->leaderboardItemex(m_renderer);
    SDL_Event *e = new SDL_Event();
    int fps = 1000 / 20;
    //anime vars
    int time = 0, total = fps / 4;
    float sc = 0;
    SDL_RenderPresent(m_renderer);
    // pop up opening anime
    while(time <= total)
    {
        sc = ease_bounceOut(0, 1.0, time, total);
        popup->pic->sc_x = sc;
        popup->pic->sc_y = sc;
        popup->setCenter(W / 2, H / 2);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        time++;
    }
    time = 0;
    total = fps / 3;
    int cl_x = W - 206, cl_y = 110, cl_st_x = W + 180, cl_fi_x = cl_x;
    int ti_x = W / 2, ti_y = 110, ti_st_y = -100, ti_en_y = ti_y;
    int bo_x = 60, bo_st_x = -1100, bo_en_x = bo_x, bo_y = 300;
    btn *close = new btn(cl_st_x, cl_y, "pic/close.png", 0.8, m_renderer);
    int ttemp;
    string title = "LeaderBoard";
    string name =  "Name";
    string goals = "Goals";
    string wins = "Wins";
    string losts = "Losts";
    string plays = "Plays";
    string scores = "Scores";
    while(time <= total)
    {
        ttemp = easeBackOut(ti_st_y, ti_en_y, time, total, 1.5);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        popup->pic->render();
        close->setXY(easeBackOut(cl_st_x, cl_fi_x, time, total, 2.5), cl_y);
        showText(ti_x, ttemp, 0, 0, "Leader Board", "fonts/GothamRounded.ttf", 60, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        textRGBA(m_renderer, 140, ttemp + 70, name.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 300, ttemp + 70, scores.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 460, ttemp + 70, goals.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 620, ttemp + 70, wins.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 780, ttemp + 70, losts.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 960, ttemp + 70, plays.c_str(), 0, 35, 0, 0, 255, 255);
        PI->leaderboardItemRenderex(easeBackIn(bo_st_x, bo_en_x, time, total, 2.5), bo_y, 0);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        time++;
    }
    int mx, my, yst = 0;
    bool in = false, ren = false;
    while(true)
    {
        SDL_PollEvent(e);
        SDL_GetMouseState(&mx, &my);
        if(e->type == SDL_MOUSEBUTTONDOWN)
        {
            if(close->btn_clicked(e->motion.x, e->motion.y))
                break;
        }

        if(popup->btn_clicked(mx, my) || in)
        {
            in = true;
            SDL_PollEvent(e);
            if(e->type == SDL_MOUSEWHEEL)
            {
                yst = max(0, yst + e->wheel.y);
                yst = min(tsize, yst + e->wheel.y);
                ren = true;
            }
        }
        if(!popup->btn_clicked(mx, my))
            in = false;
        if(ren)
        {
            ren = false;
            SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
            SDL_RenderClear(m_renderer);
            bg->render();
            popup->pic->render();
            close->setXY(easeBackOut(cl_st_x, cl_fi_x, time, total, 2.5), cl_y);
            showText(ti_x, ti_en_y, 0, 0, "Leader Board", "fonts/GothamRounded.ttf", 60, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
            textRGBA(m_renderer, 140, ti_en_y + 70, name.c_str(), 0, 35, 0, 0, 255, 255);
            textRGBA(m_renderer, 300, ti_en_y + 70, scores.c_str(), 0, 35, 0, 0, 255, 255);
            textRGBA(m_renderer, 460, ti_en_y + 70, goals.c_str(), 0, 35, 0, 0, 255, 255);
            textRGBA(m_renderer, 620, ti_en_y + 70, wins.c_str(), 0, 35, 0, 0, 255, 255);
            textRGBA(m_renderer, 780, ti_en_y + 70, losts.c_str(), 0, 35, 0, 0, 255, 255);
            textRGBA(m_renderer, 960, ti_en_y + 70, plays.c_str(), 0, 35, 0, 0, 255, 255);
            PI->leaderboardItemRenderex(bo_en_x, bo_y, yst);
            SDL_RenderPresent(m_renderer);
        }
        SDL_Delay(fps);
        e->type = 0;
    }
    time = 0;
    while(time <= total)
    {
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        popup->pic->render();
        ttemp = easeBackOut(ti_st_y, ti_en_y, time, total, 1.5);
        close->setXY(easeBackIn(cl_fi_x, cl_st_x, time, total, 2.5), cl_y);
        showText(ti_x, ttemp, 0, 0, "Leader Board", "fonts/GothamRounded.ttf", 60, SDL_Color{0, 0, 0, 255}, -1, 1, 0);
        textRGBA(m_renderer, 140, ttemp + 70, name.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 300, ttemp + 70, scores.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 460, ttemp + 70, goals.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 620, ttemp + 70, wins.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 780, ttemp + 70, losts.c_str(), 0, 35, 0, 0, 255, 255);
        textRGBA(m_renderer, 960, ttemp + 70, plays.c_str(), 0, 35, 0, 0, 255, 255);
        PI->leaderboardItemRenderex(easeBackIn(bo_en_x, bo_st_x, time, total, 2.5), bo_y, yst);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        time++;
    }
    total = fps / 4;
    time = 0;
    while(time <= total)
    {
        sc = easeBackIn(1.0, 0, time, total, 2.0);
        popup->pic->sc_x = sc;
        popup->pic->sc_y = sc;
        popup->pic->setCenter(W / 2, H / 2);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        time++;
    }
    time = 0;
}
int startMenu(playerInfo *PI)
{
    int ret = -1;
    // init start
    int fps = 1000 / 40;
    //background
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );
    string tutPath [] = {"pic/tut/1.png", "pic/tut/2.png","pic/tut/3.png"};
    int num = 3;
    num--;
    int index = 0;
    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    SDL_RenderPresent( m_renderer );
    int ic_x=920,ic_y=150,stbtn_x=900
                                  ,stbtn_y=320,rarrow_x=470,rarrow_y=280
                                  ,lerrow_x=70,lerrow_y=280,mouse_x,mouse_y;
    int tut_x = 20, tut_y = 20;
    int rank_x = 70, rank_y = 450;
    bool changeMade = true;
    //anime variables
    int st_tut_x = -725, fi_tut_x = tut_x;
    int st_rank_y = H + 400, fi_rank_y = rank_y;
    int st_ic_x = W + 150, fi_ic_x = ic_x;
    int st_st_x = W + 200, fi_st_x = stbtn_x;
    int st_leftarr_y = H + 150, fi_leftarr_y = lerrow_y; // left row
    int st_rightarr_y = H + 150, fi_rightarr_y = rarrow_y; // right row
    int time = 0, total = fps / 2;
    //end anime var
    SDL_Event *e = new SDL_Event();
    btn *stbtn = new btn(stbtn_x, stbtn_y, "pic/startbtn.png", 200.0 / 512, m_renderer);
    btn *setbtn = new btn(stbtn_x, stbtn_y + 100, "pic/settingbtn.png", 200.0 / 512, m_renderer);
    btn *ic = new btn(ic_x, ic_y, "pic/icon.jpg", 0.5, m_renderer);
    btn *leftarr = new btn(lerrow_x,lerrow_y, "pic/leftarrow.png", 225.0 / 512, m_renderer);
    btn *rightarr = new btn(rarrow_x, rarrow_y,"pic/rightarrow.png", 225.0 / 512, m_renderer);
    image *tut[3] {new image(tutPath[0], st_tut_x, tut_y, 1.0, m_renderer),
              new image(tutPath[1], st_tut_x, tut_y, 1.0, m_renderer),
              new image(tutPath[2], st_tut_x, tut_y, 1.0, m_renderer)
    };
    btn *rank = new btn(rank_x, rank_y, "pic/scoreboard.png", 0.66, m_renderer);
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );
    bg->render();
    int tsize = PI->leaderboardItem(m_renderer);
    //init end
    while(time <= total)
    {
        stbtn->setXY(easeBackOut(st_st_x, fi_st_x, time, total, 1.5), stbtn_y);
        setbtn->setXY(easeBackOut(st_st_x, fi_st_x, time, total, 1.5), stbtn_y + 100);
        ic->setXY(easeBackOut(st_ic_x, fi_ic_x, time, total, 1.5), ic_y);
        leftarr->setXY(lerrow_x, easeBackOut(st_leftarr_y, fi_leftarr_y, time, total, 1.5));
        rightarr->setXY(rarrow_x, easeBackOut(st_rightarr_y, fi_rightarr_y, time, total, 1.5));
        tut[0]->x = easeBackOut(st_tut_x, fi_tut_x, time, total, 1.5);
        tut[1]->x = easeBackOut(st_tut_x, fi_tut_x, time, total, 1.5);
        tut[2]->x = easeBackOut(st_tut_x, fi_tut_x, time, total, 1.5);
        rank->setXY(rank_x, easeBackOut(st_rank_y, fi_rank_y, time, total, 1.5));
        tut[0]->load();
        tut[1]->load();
        tut[2]->load();
        SDL_Event *e = new SDL_Event();
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
        time++;
    }
    PI->leaderboardItem(m_renderer);
    //PI->leaderboardItemRender(200, 470);
    // craete the texture here for rank item
    // events handling
    bool ren = true, setren = false, stren = false, icoren = false, lren = false, rren = false, in = false;
    int mx, my;
    int yst = 0;
    while(true)
    {
        SDL_PollEvent(e);
        SDL_PumpEvents();
        SDL_GetMouseState(&mx, &my);
        // hover
        if(stbtn->btn_clicked(mx, my))
        {
            stbtn->pic->sc_x = 220.0 / 512;
            stbtn->pic->sc_y = 220.0 / 512;
            ren = true;
            stren = true;
        }
        else if(stren)
        {
            stbtn->pic->sc_x = 200.0 / 512;
            stbtn->pic->sc_y = 200.0 / 512;
            ren = true;
            stren = false;
        }
        if(setbtn->btn_clicked(mx, my))
        {
            setbtn->pic->sc_x = 220.0 / 512;
            setbtn->pic->sc_y = 220.0 / 512;
            ren = true;
            setren = true;
        }
        else if(setren)
        {
            setbtn->pic->sc_x = 200.0 / 512;
            setbtn->pic->sc_y = 200.0 / 512;
            ren = true;
            setren = false;
        }
        if(leftarr->btn_clicked(mx, my))
        {
            leftarr->pic->sc_x = 250.0 / 512;
            leftarr->pic->sc_y = 250.0 / 512;
            leftarr->pic->load();
            ren = true;
            lren = true;
        }
        else if(lren)
        {
            leftarr->pic->sc_x = 225.0 / 512;
            leftarr->pic->sc_y = 225.0 / 512;
            leftarr->pic->load();
            ren = true;
            lren = false;
        }
        if(rightarr->btn_clicked(mx, my))
        {
            rightarr->pic->sc_x = 250.0 / 512;
            rightarr->pic->sc_y = 250.0 / 512;
            rightarr->pic->load();
            ren = true;
            rren = true;
        }
        else if(rren)
        {
            rightarr->pic->sc_x = 225.0 / 512;
            rightarr->pic->sc_y = 225.0 / 512;
            rightarr->pic->load();
            ren = true;
            rren = false;
        }
        if(ic->btn_clicked(mx, my))
        {
            ic->pic->sc_x = 0.6;
            ic->pic->sc_y = 0.6;
            ic->pic->load();
            ren = true;
            icoren = true;
        }
        else if(icoren)
        {
            ic->pic->sc_x = 0.5;
            ic->pic->sc_y = 0.5;
            ic->pic->load();
            ren = true;
            icoren = false;
        }
        // click handle
        if(e->type == SDL_MOUSEBUTTONDOWN && e->button.clicks == 1)
        {
            if(stbtn->btn_clicked(mx, my))
            {
                // start the game
                ret = 2;
                break;
            }
            if(setbtn->btn_clicked(mx, my))
            {
                // setting menu
                settingPage();
            }
            if(leftarr->btn_clicked(mx, my))
            {
                // next slide
                index++;
                index = min(index, num);
                ren = true;
            }
            if(rightarr->btn_clicked(mx, my))
            {
                // pre slide
                index--;
                index = max(0, index);
                ren = true;
            }
            if(ic->btn_clicked(mx, my))
            {
                // on icon click
                aboutUs();
            }
        }
        if(rank->btn_clicked(mx, my) || in)
        {
            in = true;
            SDL_PollEvent(e);
            if(e->type == SDL_MOUSEBUTTONDOWN)
                scorePopUp(PI);
            if(e->type == SDL_MOUSEWHEEL)
            {
                yst = max(0, yst + e->wheel.y);
                yst = min(tsize, yst + e->wheel.y);
                //cout<<tsize<<" "<<yst<<endl;
                ren = true;
            }
        }
        if(!rank->btn_clicked(mx, my))
            in = false;

        if(ren)
        {
            SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
            SDL_RenderClear( m_renderer );
            bg->render();
            rank->pic->load();
            tut[index]->load();
            stbtn->pic->load();
            setbtn->pic->load();
            ic->pic->load();
            leftarr->pic->load();
            rightarr->pic->load();
            PI->leaderboardItemRender(100, 490, yst);
            SDL_Delay(fps);
            SDL_RenderPresent( m_renderer );
            ren = false;
        }
        else
            SDL_Delay(fps);
        e->type = 0;
    }
    delete bg;
    delete e;
    delete stbtn;
    delete ic;
    delete leftarr;
    delete rightarr;
    delete tut;
    delete rank;
    return ret;
}

int endMenu(playerInfo *PI, int score1, int score2)
{
    int ret = -1;
    int fps = 1000 / 40;
    //background
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );
    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    // vars for anime
    int total = fps / 2, time = 0;
    int ic_x = W / 2 - 75, ic_st_y = -80, ic_fi_y = 50;
    int pname1_x = W / 4, pname1_st_x = -200, pname1_fi_x = pname1_x, pname1_y = 150;
    int score1_x = W / 4, score1_st_x = -200, score1_fi_x = score1_x, score1_y = 220;
    int bscore1_x = W / 4, bscore1_st_x = -200, bscore1_fi_x = bscore1_x, bscore1_y = 290;
    int pname2_x = 3 * W / 4, pname2_st_x = W + 200, pname2_fi_x = pname2_x, pname2_y = 150;
    int score2_x = 3 * W / 4, score2_st_x = W + 200, score2_fi_x = score2_x, score2_y = 220;
    int bscore2_x = 3 * W / 4, bscore2_st_x = W + 200, bscore2_fi_x = bscore2_x, bscore2_y = 290;
    int pl_x = W / 2, pl_y = 320, pl_st_y = H + 50, pl_fi_y = pl_y;
    int ho_x = W / 2, ho_y = 500, ho_st_y = H + 100, ho_fi_y = ho_y;
    int ex_x = W / 2, ex_y = 680, ex_st_y = H + 150, ex_fi_y = ex_y;
    float ic_sc = 0.5;
    image *ic = new image("pic/icon.jpg", ic_x, ic_st_y, ic_sc, m_renderer);
    btn *pl_btn = new btn(pl_x, pl_st_y, "pic/playbtn.png", 0.6, m_renderer);
    pl_btn->setCenter(pl_x, pl_st_y);
    btn *ho_btn = new btn(ho_x, ho_st_y, "pic/homebtn.png", 0.6, m_renderer);
    pl_btn->setCenter(ho_x, ho_st_y);
    btn *ex_btn = new btn(ex_x, ex_st_y, "pic/exitbtn.png", 0.6, m_renderer);
    pl_btn->setCenter(ex_x, ex_st_y);
    SDL_Color cl;
    cl.r = 255;
    cl.g = 255;
    cl.b = 255;
    cl.a = 255;
    SDL_RenderPresent(m_renderer);
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );
    bg->render();
    if(PI->p1.score < score1)
    {
        PI->p1.score = score1;
        PI->update(PI->p1.name, score1);
    }
    if(PI->p2.score < score2)
    {
        PI->p2.score = score2;
        PI->update(PI->p2.name, score2);
    }
    PI->p1.goals + score1;
    PI->p2.goals + score2;
    PI->p1.played++;
    PI->p2.played++;
    if(score1 > score2)
    {
        PI->p1.wins++;
        PI->p2.losts++;
    }
    else if(score2 > score1)
    {
        PI->p2.wins++;
        PI->p1.losts++;
    }
    while(time <= total)
    {
        ic->y = ease_bounceOut(ic_st_y, ic_fi_y, time, total);
        ic->load();
        showText(easeBackOut(pname1_st_x, pname1_fi_x, time, total, 2.0), pname1_y, 0, 0, PI->p1.name, "fonts/Lazyfont.ttf", 60, cl, 0, 1, 0);
        showText(easeBackOut(score1_st_x, score1_fi_x, time, total, 2.0), score1_y, 0, 0, to_string(score1), "fonts/Lazyfont.ttf", 45, SDL_Color{0, 10, 255, 255}, 0, 1, 0);
        showText(easeBackOut(bscore1_st_x, bscore1_fi_x, time, total, 2.0), bscore1_y, 0, 0, to_string(PI->p1.score), "fonts/Lazyfont.ttf", 37, SDL_Color{0, 10, 255, 255}, 0, 1, 0);
        showText(easeBackOut(pname2_st_x, pname2_fi_x, time, total, 2.0), pname2_y, 0, 0, PI->p2.name, "fonts/Lazyfont.ttf", 60, cl, 0, 1, 0);
        showText(easeBackOut(score2_st_x, score2_fi_x, time, total, 2.0), score2_y, 0, 0, to_string(score2), "fonts/Lazyfont.ttf", 45, SDL_Color{0, 10, 255, 255}, 0, 1, 0);
        showText(easeBackOut(bscore2_st_x, bscore2_fi_x, time, total, 2.0), bscore2_y, 0, 0, to_string(PI->p2.score), "fonts/Lazyfont.ttf", 37, SDL_Color{0, 10, 255, 255}, 0, 1, 0);
        pl_btn->setCenter(pl_x, easeBackOut(pl_st_y, pl_fi_y, time, total, 2.0));
        ho_btn->setCenter(ho_x, easeBackOut(ho_st_y, ho_fi_y, time, total, 2.0));
        ex_btn->setCenter(ex_x, easeBackOut(ex_st_y, ex_fi_y, time, total, 2.0));
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
        time++;
    }
    SDL_Event *e = new SDL_Event();
    bool ren = true, plren = false, horen = false, exren = false;
    int mx, my;
    while(true)
    {
        SDL_PollEvent(e);
        mx = e->motion.x;
        my = e->motion.y;
        // bover
        if(pl_btn->btn_clicked(mx, my))
        {
            pl_btn->pic->sc_x = .7;
            pl_btn->pic->sc_y = .7;
            pl_btn->setCenter(pl_x, easeBackOut(pl_st_y, pl_fi_y, time, total, 2.0));
            ren = true;
            plren = true;
        }
        else if(plren)
        {
            pl_btn->pic->sc_x = .6;
            pl_btn->pic->sc_y = .6;
            pl_btn->setCenter(pl_x, easeBackOut(pl_st_y, pl_fi_y, time, total, 2.0));
            plren = false;
            ren = true;
        }
        if(ho_btn->btn_clicked(mx, my))
        {
            ho_btn->pic->sc_x = .7;
            ho_btn->pic->sc_y = .7;
            ho_btn->setCenter(ho_x, easeBackOut(ho_st_y, ho_fi_y, time, total, 2.0));
            ren = true;
            horen = true;
        }
        else if(horen)
        {
            ho_btn->pic->sc_x = .6;
            ho_btn->pic->sc_y = .6;
            ho_btn->setCenter(ho_x, easeBackOut(ho_st_y, ho_fi_y, time, total, 2.0));
            horen = false;
            ren = true;
        }
        if(ex_btn->btn_clicked(mx, my))
        {
            ex_btn->pic->sc_x = .7;
            ex_btn->pic->sc_y = .7;
            ex_btn->setCenter(ex_x, easeBackOut(ex_st_y, ex_fi_y, time, total, 2.0));
            ren = true;
            exren = true;
        }
        else if(exren)
        {
            ex_btn->pic->sc_x = .6;
            ex_btn->pic->sc_y = .6;
            ex_btn->setCenter(ex_x, easeBackOut(ex_st_y, ex_fi_y, time, total, 2.0));
            exren = false;
            ren = true;
        }

        if(e->type == SDL_MOUSEBUTTONDOWN && e->button.clicks == 1)
        {
            if(pl_btn->btn_clicked(mx, my))
            {
                ret = 2;
                break;
            }
            if(ho_btn->btn_clicked(mx, my))
            {
                ret = 1;
                break;
            }
            if(ex_btn->btn_clicked(mx, my))
            {
                ret = -1;
                break;
            }
        }
        if(ren)
        {
            SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
            SDL_RenderClear( m_renderer );
            bg->render();
            ic->render();
            showText(pname1_x, pname1_y, 0, 0, PI->p1.name, "fonts/Lazyfont.ttf", 60, cl, 0, 1, 0);
            showText(pname2_x, pname2_y, 0, 0, PI->p2.name, "fonts/Lazyfont.ttf", 60, cl, 0, 1, 0);
            showText(score1_x, score1_y, 0, 0, to_string(score1), "fonts/Lazyfont.ttf", 45, SDL_Color{0, 10, 255, 255}, 0, 1, 0);
            showText(score2_x, score2_y, 0, 0, to_string(score2), "fonts/Lazyfont.ttf", 45, SDL_Color{0, 10, 255, 255}, 0, 1, 0);
            showText(bscore1_x, bscore1_y, 0, 0, to_string(PI->p1.score), "fonts/Lazyfont.ttf", 37, SDL_Color{0, 10, 255, 255}, 0, 1, 0);
            showText(bscore2_x, bscore2_y, 0, 0, to_string(PI->p2.score), "fonts/Lazyfont.ttf", 37, SDL_Color{0, 10, 255, 255}, 0, 1, 0);
            pl_btn->setCenter(pl_x, pl_y);
            ho_btn->setCenter(ho_x, ho_y);
            ex_btn->setCenter(ex_x, ex_y);
            SDL_RenderPresent(m_renderer);
            ren = false;
        }
        SDL_Delay(fps);
    }
    delete bg;
    delete ic;
    delete pl_btn;
    delete ho_btn;
    delete ex_btn;
    delete e;
    PI->save_list();
    return ret;
}

setting select(playerInfo *PI)
{
    struct setting st;
    int fps = 1000 / 20;
    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    SDL_RenderClear(m_renderer);
    //anime
    int bg_x = 0, bg_y = 50, bg_st_x = -1280, bg_fi_x = bg_x;
    int he_x = 0, he_y = 270, he_fi_x = he_x;
    int bo_x = 0, bo_y = 430, bo_fi_x = bo_x;
    int time = 0, total = fps / 2;
    //
    btn *bgx[4];
    btn *head[12];
    btn *body[5];
    for(int i = 0; i < 4; i++)
        bgx[i] = new btn(bg_st_x + 136 * (i + 1) + 150 * i, bg_y, "pic/bg/" + to_string(i + 1) + ".png", 1.0, m_renderer);
    for(int i = 0; i < 12; i++)
        head[i] = new btn(bg_st_x + 25 * (i + 1) + 80 * i, he_y, "pic/head/" + to_string(i + 1) + ".png", .8, m_renderer);
    for(int i = 0; i < 5; i++)
        body[i] = new btn(bg_st_x + 170 * (i + 1) + 52 * i, he_y, "pic/body/" + to_string(i + 1) + ".png", 1.0, m_renderer);
    while(time <= total)
    {
        bg_x = easeBackOut(bg_st_x, bg_fi_x, time, total, 1.5);
        for(int i = 0; i < 4; i++)
            bgx[i]->setXY(bg_x + 136 * (i + 1) + 150 * i, bg_y);
        for(int i = 0; i < 12; i++)
            head[i]->setXY(bg_x + 25 * (i + 1) + 80 * i, he_y);
        for(int i = 0; i < 5; i++)
            body[i]->setXY(bg_x + 170 * (i + 1) + 52 * i, bo_y);
        SDL_RenderPresent(m_renderer);
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear(m_renderer);
        bg->render();
        time++;
        //cout<<bg_x<<"\t";
    }
    bool flag = false;
    SDL_Event *e = new SDL_Event();
    int mx, my;

    while(!flag)
    {
        SDL_PollEvent(e);
        mx = e->motion.x;
        my = e->motion.y;
        //hover
        if(e->type == SDL_MOUSEBUTTONDOWN)
        {
            for(int i = 0; i < 4; i++)
            {
                if(bgx[i]->btn_clicked(mx, my))
                {
                    st.bg = i+1;
                    bgx[i]->pic->sc_x = 1.2;
                    bgx[i]->pic->sc_y = 1.2;
                    bgx[i]->pic->load();
                    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
                    SDL_RenderClear( m_renderer );
                    bg->render();
                    for(int i = 0; i < 4; i++)
                        bgx[i]->pic->render();
                    for(int i = 0; i < 12; i++)
                        head[i]->pic->render();
                    for(int i = 0; i < 5; i++)
                        body[i]->pic->render();
                    SDL_RenderPresent(m_renderer);
                    flag = true;
                    break;
                }
            }
            SDL_Delay(fps);
        }
    }
    e->type = 0;
    flag = false;
    while(!flag)
    {
        //cout<<"2"<<endl;
        SDL_PollEvent(e);
        mx = e->motion.x;
        my = e->motion.y;
        //hover
        if(e->type == SDL_MOUSEBUTTONDOWN)
        {
            for(int i = 0; i < 12; i++)
            {
                if(head[i]->btn_clicked(mx, my))
                {
                    st.head = i+1;
                    head[i]->pic->sc_x = 1.1;
                    head[i]->pic->sc_y = 1.1;
                    head[i]->pic->load();
                    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
                    SDL_RenderClear( m_renderer );
                    bg->render();
                    for(int i = 0; i < 4; i++)
                        bgx[i]->pic->render();
                    for(int i = 0; i < 12; i++)
                        head[i]->pic->render();
                    for(int i = 0; i < 5; i++)
                        body[i]->pic->render();
                    SDL_RenderPresent(m_renderer);
                    flag = true;
                    break;
                }
            }
        }
        SDL_Delay(fps);
    }
    flag = false;
    e->type = 0;
    while(!flag)
    {
        SDL_PollEvent(e);
        mx = e->motion.x;
        my = e->motion.y;
        //hover
        if(e->type == SDL_MOUSEBUTTONDOWN)
        {
            for(int i = 0; i < 5; i++)
            {
                if(body[i]->btn_clicked(mx, my))
                {
                    st.body = i+1;
                    body[i]->pic->sc_x = 1.2;
                    body[i]->pic->sc_y = 1.2;
                    body[i]->pic->load();
                    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
                    SDL_RenderClear( m_renderer );
                    bg->render();
                    for(int i = 0; i < 4; i++)
                        bgx[i]->pic->render();
                    for(int i = 0; i < 12; i++)
                        head[i]->pic->render();
                    for(int i = 0; i < 5; i++)
                        body[i]->pic->render();
                    SDL_RenderPresent(m_renderer);
                    flag = true;
                    break;
                }
            }
        }
        SDL_Delay(fps);
    }
    e->type = NULL;
    string sec = "";
    // text input one
    SDL_StartTextInput();
    time = 1;
    while(true)
    {
        SDL_PollEvent(e);
        if(e->key.keysym.sym == SDLK_RETURN)
            break;
        if(time % 5 == 0)
        {
            time = 1;
            if(sec[sec.length() - 1] == '|')
            {
                sec.pop_back();
            }
            else
            {
                sec += "|";
            }
        }
        if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_BACKSPACE && sec.length() > 0)
        {
            if(sec[sec.length() - 1] == '|')
            {
                sec.pop_back();
                sec.pop_back();
                sec += '|';
            }
            else
            {
                sec.pop_back();
            }
            e->type = NULL;
        }
        if(e->type == SDL_TEXTINPUT)
        {
            if(sec[sec.length() - 1] == '|')
            {
                sec.pop_back();
                sec += e->text.text;
                sec += '|';
            }
            else
            {
                sec += e->text.text;
            }
        }
        if(sec != "")
            textRGBA(m_renderer, W / 2, 600, sec.c_str(), 1, 45, 255, 255, 255, 255);
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
        for(int i = 0; i < 4; i++)
            bgx[i]->pic->render();
        for(int i = 0; i < 12; i++)
            head[i]->pic->render();
        for(int i = 0; i < 5; i++)
            body[i]->pic->render();
        SDL_RenderPresent(m_renderer);
        time++;
    }
    if(sec[sec.length() - 1] == '|')
        sec.pop_back();
    st.sec = stoi(sec);
    return st;
}

setting newSelect(playerInfo *PI, int nhe)
{
    struct setting st;
//    int fps = 1000 / 20;
    int fps = 24;
    image *bg = new image("pic/inputbg.jpg", 0, 0, 1.0, m_renderer);
    bg->sc_x = 1.5023474178403755868544600938967;
    bg->sc_y = 1.6666666666666666666666666666667;
    bg->load();
    SDL_RenderClear(m_renderer);
    int ptr_x = W - 200, ptr_st_x = W + 200, ptr_en_x = ptr_x, ptr_y = 200 - 20;
    int nhead = 12 + nhe,nbody = 5,nbg = 4, nball = 4, ball_index = 0, bg_index = 0, head_index = 0, body_index = 0;
    int total = fps / 2, time = 0;
    btn *ptr = new btn(ptr_st_x, ptr_y, "pic/leftarrow.png", 0.2, m_renderer);
    btn *bgx[nbg];
    btn *head[nhead];
    btn *body[nbody];
    btn *ball[nball];

    int bg_x = -200, bg_st_x = bg_x, bg_en_x = W / 2, bg_out_x = W + 200, bg_y = 200 - 20;
    int he_x = -250, he_st_x = he_x, he_en_x = W / 2, he_out_x = W + 250, he_y = 300 + 100 - 20 - 15;
    int bd_x = -250, bd_st_x = bd_x, bd_en_x = W / 2, bd_out_x = W + 250, bd_y = 300 + 100 + 215 - 20 - 15;
    int bl_x = W / 4 - 40, bl_y = -80, bl_st_y = bl_y, bl_en_y = H / 2, bl_out_y = H + 80;
    for(int i = 0; i < nbg; i++)
        bgx[i] = new btn(bg_st_x, bg_y, "pic/bg/" + to_string(i + 1) + ".png", 1.0, m_renderer);
    for(int i = 0; i < nhead; i++)
        head[i] = new btn(he_st_x, he_y, "pic/head/" + to_string(i + 1) + ".png", 2.0, m_renderer);
    for(int i = 0; i < nbody; i++)
        body[i] = new btn(bd_st_x, bd_y, "pic/body/" + to_string(i + 1) + ".png", 2.0, m_renderer);
    for(int i = 0; i < nball; i++)
        ball[i] = new btn(bl_x, bl_st_y, "pic/ball/" + to_string(i + 1) + ".png", 1.0, m_renderer);
    // 0 -> left 1 -> right
    bool flag = false, ren = false, dir = 0;
    // select bg
    while(time <= total)
    {
        ptr->setCenter(easeBackOut(ptr_st_x, ptr_en_x, time, total, 1.5), ptr_y);
        bgx[0]->setCenter(easeBackOut(bg_st_x, bg_en_x, time, total, 1.5), bg_y);
        head[0]->setCenter(easeBackOut(he_st_x, he_en_x, time, total, 1.5), he_y);
        body[0]->setCenter(easeBackOut(bd_st_x, bd_en_x, time, total, 1.5), bd_y);
        ball[0]->setCenter(bl_x, easeBackOut(bl_st_y, bl_en_y, time, total, 1.5));
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
        showText(W / 2, 60, 0, 0, "Use arrow key to select what U want :)", "fonts/BM.ttf", 60, SDL_Color{255, 255, 255, 255}, 0, 1, 0);
        time++;
    }
    SDL_Event *e = new SDL_Event();
    int stat = 0;
    while(!flag)
    {
        while(stat == 0 && !flag)
        {
            ptr->setCenter(ptr_x, bg_y);
            if(time <= total)
            {
                ren = true;
                if(dir == 0)
                {
                    bgx[bg_index + 1]->setCenter(easeBackIn(bg_en_x, bg_st_x, time, total / 2, .75), bg_y);
                    bgx[bg_index]->setCenter(easeBackOut(bg_out_x, bg_en_x, time, total, 1.5), bg_y);
                }
                else
                {
                    bgx[bg_index - 1]->setCenter(easeBackIn(bg_en_x, bg_out_x, time, total / 2, .75), bg_y);
                    bgx[bg_index]->setCenter(easeBackOut(bg_st_x, bg_en_x, time, total, 1.5), bg_y);
                }
                time++;
            }
            else
            {
                SDL_PollEvent(e);
                if(e->key.keysym.sym == SDLK_RIGHT && e->type == SDL_KEYDOWN)
                {
                    if(bg_index + 1 < nbg)
                    {
                        dir = 1;
                        bg_index++;
                        time = 0;
                    }
                }
                else if(e->key.keysym.sym == SDLK_LEFT && e->type == SDL_KEYDOWN)
                {
                    if(bg_index - 1 >= 0)
                    {
                        dir = 0;
                        bg_index--;
                        time = 0;
                    }
                }
                else if(e->key.keysym.sym == SDLK_RETURN && e->type == SDL_KEYDOWN)
                {
                    flag = true;
                    ren = true;
                }

                else if(e->key.keysym.sym == SDLK_UP && e->type == SDL_KEYDOWN)
                {
                    if(stat - 1 >= 0)
                    {
                        stat -= 1;
                        break;
                    }
                }
                else if(e->key.keysym.sym == SDLK_DOWN && e->type == SDL_KEYDOWN)
                {
                    if(stat + 1 <= 3)
                    {
                        stat += 1;
                        break;
                    }
                }
            }
            if(ren)
            {
                SDL_RenderPresent( m_renderer );
                SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
                SDL_RenderClear( m_renderer );
                bg->render();
                //bgx[bg_index]->pic->render();
                head[head_index]->pic->render();
                ball[ball_index]->pic->render();
                body[body_index]->pic->render();
//                boxColor(m_renderer,W/2-80,180-80,W/2+80,180+80,0xff2f9fb3);
                showText(W / 2, 60, 0, 0, "Use arrow key to select what U want :)", "fonts/BM.ttf", 60, SDL_Color{255, 255, 255, 255}, 0, 1, 0);
                ren = false;
            }
            e->type = 0;
            SDL_Delay(fps);
        }
        e->type = 0;
        SDL_Delay(fps);
        while(stat == 1 && !flag)
        {
            ptr->setCenter(ptr_x, he_y);
            if(time <= total)
            {
                ren = true;
                if(dir == 0)
                {
                    head[head_index + 1]->setCenter(easeBackIn(bg_en_x, bg_st_x, time, total / 2, .75), he_y);
                    head[head_index]->setCenter(easeBackOut(bg_out_x, bg_en_x, time, total, 1.5), he_y);
                }
                else
                {
                    head[head_index - 1]->setCenter(easeBackIn(bg_en_x, bg_out_x, time, total / 2, .75), he_y);
                    head[head_index]->setCenter(easeBackOut(bg_st_x, bg_en_x, time, total, 1.5), he_y);
                }
                time++;
            }
            else
            {
                SDL_PollEvent(e);
                if(e->key.keysym.sym == SDLK_RIGHT && e->type == SDL_KEYDOWN)
                {
                    if(head_index + 1 < nhead)
                    {
                        dir = 1;
                        head_index++;
                        time = 0;
                    }
                }
                else if(e->key.keysym.sym == SDLK_LEFT && e->type == SDL_KEYDOWN)
                {
                    if(head_index - 1 >= 0)
                    {
                        dir = 0;
                        head_index--;
                        time = 0;
                    }
                }
                else if(e->key.keysym.sym == SDLK_RETURN && e->type == SDL_KEYDOWN)
                {
                    flag = true;
                    ren = true;
                }
                else if(e->key.keysym.sym == SDLK_UP && e->type == SDL_KEYDOWN)
                {
                    if(stat - 1 >= 0)
                    {
                        stat -= 1;
                        break;
                    }
                }
                else if(e->key.keysym.sym == SDLK_DOWN && e->type == SDL_KEYDOWN)
                {
                    if(stat + 1 <= 3)
                    {
                        stat += 1;
                        break;
                    }
                }
            }
            if(ren)
            {
                SDL_RenderPresent( m_renderer );
                SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
                SDL_RenderClear( m_renderer );
                bg->render();
                bgx[bg_index]->pic->render();
                //head[head_index]->pic->render();
                body[body_index]->pic->render();
                ball[ball_index]->pic->render();
                showText(W / 2, 60, 0, 0, "Use arrow key to select what U want :)", "fonts/BM.ttf", 60, SDL_Color{255, 255, 255, 255}, 0, 1, 0);
                ren = false;
            }
            e->type = 0;
            SDL_Delay(fps);
        }
        e->type = 0;
        SDL_Delay(fps);
        while(stat == 2 && !flag)
        {
            ptr->setCenter(ptr_x, bd_y);
            if(time <= total)
            {
                ren = true;
                if(dir == 0)
                {
                    body[body_index + 1]->setCenter(easeBackIn(bg_en_x, bg_st_x, time, total / 2, .75), bd_y);
                    body[body_index]->setCenter(easeBackOut(bg_out_x, bg_en_x, time, total, 1.5), bd_y);
                }
                else
                {
                    body[body_index - 1]->setCenter(easeBackIn(bg_en_x, bg_out_x, time, total / 2, .75), bd_y);
                    body[body_index]->setCenter(easeBackOut(bg_st_x, bg_en_x, time, total, 1.5), bd_y);
                }
                time++;
            }
            else
            {
                SDL_PollEvent(e);
                if(e->key.keysym.sym == SDLK_RIGHT && e->type == SDL_KEYDOWN)
                {
                    if(body_index + 1 < nbody)
                    {
                        dir = 1;
                        body_index++;
                        time = 0;
                    }
                }
                else if(e->key.keysym.sym == SDLK_LEFT && e->type == SDL_KEYDOWN)
                {
                    if(body_index - 1 >= 0)
                    {
                        dir = 0;
                        body_index--;
                        time = 0;
                    }
                }
                else if(e->key.keysym.sym == SDLK_RETURN && e->type == SDL_KEYDOWN)
                {
                    flag = true;
                    ren = true;
                }
                else if(e->key.keysym.sym == SDLK_UP && e->type == SDL_KEYDOWN)
                {
                    if(stat - 1 >= 0)
                    {
                        stat -= 1;
                        break;
                    }
                }
                else if(e->key.keysym.sym == SDLK_DOWN && e->type == SDL_KEYDOWN)
                {
                    if(stat + 1 <= 3)
                    {
                        stat += 1;
                        break;
                    }
                }
            }
            if(ren)
            {
                SDL_RenderPresent( m_renderer );
                SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
                SDL_RenderClear( m_renderer );
                bg->render();
                bgx[bg_index]->pic->render();
                head[head_index]->pic->render();
                ball[ball_index]->pic->render();
                //body[body_index]->pic->render();
                showText(W / 2, 60, 0, 0, "Use arrow key to select what U want :)", "fonts/BM.ttf", 60, SDL_Color{255, 255, 255, 255}, 0, 1, 0);
                ren = false;
            }
            e->type = 0;
            SDL_Delay(fps);
        }
        e->type = 0;
        SDL_Delay(fps);
        while(stat == 3 && !flag)
        {
            ptr->setCenter(400, H / 2);
            if(time <= total)
            {
                ren = true;
                if(dir == 0)
                {
                    ball[ball_index + 1]->setCenter(bl_x,easeBackIn(bl_en_y, bl_st_y, time, total / 2, .75));
                    ball[ball_index]->setCenter(bl_x, easeBackOut(bl_out_y, bl_en_y, time, total, 1.5));
                }
                else
                {
                    ball[ball_index - 1]->setCenter(bl_x, easeBackIn(bl_en_y, bl_out_y, time, total / 2, .75));
                    ball[ball_index]->setCenter(bl_x, easeBackOut(bl_st_y, bl_en_y, time, total, 1.5));
                }
                time++;
            }
            else
            {
                SDL_PollEvent(e);
                if(e->key.keysym.sym == SDLK_RIGHT && e->type == SDL_KEYDOWN)
                {
                    if(ball_index + 1 < nball)
                    {
                        dir = 1;
                        ball_index++;
                        time = 0;
                    }
                }
                else if(e->key.keysym.sym == SDLK_LEFT && e->type == SDL_KEYDOWN)
                {
                    if(ball_index - 1 >= 0)
                    {
                        dir = 0;
                        ball_index--;
                        time = 0;
                    }
                }
                else if(e->key.keysym.sym == SDLK_RETURN && e->type == SDL_KEYDOWN)
                    flag = true;
                else if(e->key.keysym.sym == SDLK_UP && e->type == SDL_KEYDOWN)
                {
                    if(stat - 1 >= 0)
                    {
                        stat -= 1;
                        break;
                    }
                }
                else if(e->key.keysym.sym == SDLK_DOWN && e->type == SDL_KEYDOWN)
                {
                    if(stat + 1 <= 3)
                    {
                        stat += 1;
                        break;
                    }
                }
            }
            if(ren)
            {
                SDL_RenderPresent( m_renderer );
                SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
                SDL_RenderClear( m_renderer );
                bg->render();
                bgx[bg_index]->pic->render();
                head[head_index]->pic->render();
                body[body_index]->pic->render();
                showText(W / 2, 60, 0, 0, "Use arrow key to select what U want :)", "fonts/BM.ttf", 60, SDL_Color{255, 255, 255, 255}, 0, 1, 0);
                ren = false;
            }
            e->type = 0;
            SDL_Delay(fps);
        }
    }

    fps=1000/20;
    // get the time input
    e->type = NULL;
    string sec = "";
    // text input one
    SDL_StartTextInput();
    time = 1;
    string ttxt = "Enter time interval : ";
    while(true)
    {
        SDL_PollEvent(e);
        if(e->key.keysym.sym == SDLK_RETURN && e->type == SDL_KEYDOWN)
            break;
        if(time % 5 == 0)
        {
            time = 1;
            if(sec[sec.length() - 1] == '|')
            {
                sec.pop_back();
            }
            else
            {
                sec += "|";
            }
        }
        if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_BACKSPACE && sec.length() > 0)
        {
            if(sec[sec.length() - 1] == '|')
            {
                sec.pop_back();
                sec.pop_back();
                sec += '|';
            }
            else
            {
                sec.pop_back();
            }
            e->type = NULL;
        }
        if(e->type == SDL_TEXTINPUT)
        {
            if(sec[sec.length() - 1] == '|')
            {
                sec.pop_back();
                sec += e->text.text;
                sec += '|';
            }
            else
            {
                sec += e->text.text;
            }
        }
        if(sec != "")
            textRGBA(m_renderer, W / 2, 720, sec.c_str(), 1, 45, 255, 255, 255, 255);
        SDL_RenderPresent( m_renderer );
        SDL_Delay(fps);
        SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
        SDL_RenderClear( m_renderer );
        bg->render();
        bgx[bg_index]->pic->render();
        head[head_index]->pic->render();
        body[body_index]->pic->render();
        ball[ball_index]->pic->render();
        showText(W / 2, 60, 0, 0, "Use arrow key to select what U want :)", "fonts/BM.ttf", 60, SDL_Color{255, 255, 255, 255}, 0, 1, 0);

        textRGBA(m_renderer, 40, 720, ttxt.c_str(), 1, 45, 255, 255, 255, 255);
        SDL_RenderPresent(m_renderer);
        time++;
    }
    if(sec[sec.length() - 1] == '|')
        sec.pop_back();
    st.sec = stoi(sec);
    st.bg = bg_index + 1;
    st.head = head_index + 1;
    st.body = body_index + 1;
    st.ball = ball_index + 1;
    return st;
}

void showText(int x, int y, int width, int height, string text, string fontName, int size, SDL_Color color, int alignVertical, int alignHorizontal, int angle)
{
    TTF_Init();
    TTF_Font *font = TTF_OpenFont((fontName).c_str(), size);
    int textWidth, textHeight;
    TTF_SizeText(font, text.c_str(), &textWidth, &textHeight);
    switch (alignHorizontal)
    {
    case 0:
        x += width - textWidth;
        break;

    case 1:
        x += (width - textWidth) / 2;
        break;
    }
    switch (alignVertical)
    {
    case 0:
        y += (height - textHeight) / 2;
        break;

    case 1:
        y += (height - textHeight);
        break;
    }

    SDL_Rect rectText{x, y, width, height};
    SDL_Surface *textSur = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture *textTex = SDL_CreateTextureFromSurface(m_renderer, textSur);
    SDL_FreeSurface(textSur);
    SDL_QueryTexture(textTex, nullptr, nullptr, &rectText.w, &rectText.h);
    SDL_RenderCopyEx(m_renderer, textTex, nullptr, &rectText, angle, NULL, SDL_FLIP_NONE);
    SDL_DestroyTexture(textTex);
    TTF_CloseFont(font);
}

float ease_bounceOut(float start, float final, int time, int total)
{
    float c = final - start; // change
    float t = float(time) / total;
    if(t < (1 / 2.75))
    {
        return c * (7.5625 * t * t) + start;
    }
    else if(t < (2 / 2.75))
    {
        t -= (1.5 / 2.75);
        return c * (7.5625 * t * t + .75) + start;
    }
    else if(t < (2.5 / 2.75))
    {
        t -= (2.25 / 2.75);
        return c * (7.5625 * t * t + .9375) + start;
    }
    else
    {
        t -= (2.625 / 2.75);
        return c * (7.5625 * t * t + .984375) + start;
    }
}

float ease_circ_in(int start_point, int end_point, int current_time, int total_time)
{
    int delta = end_point - start_point;
    double t = (double)current_time / (double)total_time;
    return (-1.0 * delta * (sqrt(1.0 - t * t) - 1.0) + start_point);
}
float easeBackOut(float p1, float p2, int time, int totalTime, float overshoot = 1.0)
{
    //opposite of easeInBack; this function implements movement that starts fast and slows down going past the ending keyframe
    //implements a bounce in the animation
    //max velocity in the start
    //By Borna Khodabandeh
    int d = p2 - p1;
    double s = overshoot;
    double t = time / (double)totalTime - 1;
    return d * (t * t * ((s + 1) * t + s) + 1) + p1;
}

float easeBackIn(float p1, float p2, int time, int totalTime, float overshoot = 1.0)
{
    //a function that implements an animation that goes past initial frame and then slowly accelerates ball as it reaches the end
    //this function implements a bounce in the animation
    //max velocity as it reaches the end
    //By Borna Khodabandeh

    float d = p2 - p1;
    double s = overshoot;
    double t = time / (double)totalTime;
    return d * t * t * ((s + 1) * t - s) + p1;
}

void surprise()
{
    Mix_HaltMusic();
    SDL_Delay(500);
    Mix_Music *s_music=Mix_LoadMUS("./pfft.mp3");
    Mix_PlayMusic(s_music,-1);
    // Clear the window with a black background
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    // Show the window
    SDL_RenderPresent(m_renderer);
    SDL_Event *e = new SDL_Event();
    // Wait for a key to be pressed
    minigame game = minigame(10, 5);
    mBall Ball;
    while (e->key.keysym.sym != SDLK_ESCAPE)
    {
        SDL_PollEvent(e);
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);
        game.showMiniGame();
        Ball.render();
        if (!Ball.collision(&game))
        {
            SDL_Color color{255,255,255,255};

            showText(W/2,H/2,0,0,"Press 'R' to reset","./GothamRounded.ttf",50,color,0,1,0);
            SDL_RenderPresent(m_renderer);
            e->type=0;
            while (e->key.keysym.sym!=SDLK_r)
                SDL_PollEvent(e);
            game.reset();
            Ball.reset();
        }
        SDL_RenderPresent(m_renderer);
    }

    Mix_FreeMusic(s_music);
}
