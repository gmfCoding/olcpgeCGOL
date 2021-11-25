#include <iostream>
#include <sstream>

#include "olcPixelGameEngine.h"
#define MAPSIZE 1024
#define MAPX MAPSIZE 
#define MAPY MAPSIZE
#define MAPRATIO 1024 / MAPSIZE
#define InBound(x, y) (x >= 0 && x < MAPX) && (y >= 0 && y < MAPY)


class GameOfLife : public olc::PixelGameEngine
{
public:
    struct Cell
    {
        bool active = false;
    };
    
    bool simulate = true;
    bool simulateOnce;
    const int width = MAPX;
    const int height = MAPY;
    olc::Pixel background;

    // Note the individual size of cell is small enough for us to fit the world on the stack, but for larger data structures you may need to use heap allocated arrays
    Cell pastGrid[MAPX][MAPY];
    Cell nextGrid[MAPX][MAPY];  

private:
    float fTargetFrameTime = 1.0f / 144.0f;
    float fAccumulatedTime = 0.0f;
    // Simulate the world
    int32_t particleCount = 0;

 
    // Constructors
public:
    GameOfLife()
    {
        sAppName = "FallingSand";
    }

    // Methods
public:

    // Removes all particles from the simulation
    void ResetSimulation()
    {
        particleCount = 0;

        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                pastGrid[x][y] = Cell();
                nextGrid[x][y] = Cell(); 
            }
        }
    }

    void Randomise(int chance)
    {
        ResetSimulation();
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                int8_t r = rand() % 100;
                if (r <= chance)
                {    
                    pastGrid[x][y].active = rand() % 2 == 0;
                }
            }
        }
    }

    bool OnUserCreate() override
    {
        // Called once at the start, so create things here
        background = olc::Pixel(93, 95, 102);
        ResetSimulation();
        Randomise(50);

        return true;
    }

    int GetNeighbourCount(int x, int y)
    {
        #define REL(i, j) (InBound(x+i, y+j) && pastGrid[x+i][y+j].active)
        int count = 0;

        #define INC {count++;}
        if(REL(0,1)) INC
        if(REL(0,-1)) INC

        if(REL(1,1)) INC
        if(REL(1,-1)) INC

        if(REL(-1,1)) INC
        if(REL(-1,-1)) INC

        if(REL(1,0)) INC
        if(REL(-1,0)) INC
        #undef INC
        #undef REL

        return count;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {

        // Fixed time step, comment out to run as fast as possible
        // fAccumulatedTime += fElapsedTime;
        // if(fAccumulatedTime >= fTargetFrameTime)
        // {
        //     fAccumulatedTime -= fTargetFrameTime;
        //     fElapsedTime = fTargetFrameTime;
        // }
        // else
        // {
        //     return true;
        // }

        // Main user input control
        UserInput();

        // Clears the previous frame
        Clear(olc::Pixel(background.r, background.g, background.b));
        
        bool mapChanged = false;
         if (simulate == true || simulateOnce)
        {
            mapChanged = true;
            simulateOnce = false;

            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    Cell* self = &pastGrid[x][y];
                    Cell* newCell = &nextGrid[x][y];

                    int neighbours = GetNeighbourCount(x,y);
                    if (neighbours < 2)
                    {
                        newCell->active = false;
                    }
                    else if(neighbours == 2)
                    {
                        newCell->active = self->active;
                    }
                    else if (neighbours == 3)
                    {
                        newCell->active = true;
                    }else if (neighbours > 3) {
                        newCell->active = false;
                    }
                    
                }
            }
        }

        // Render the world while clearing the nextGrid buffer
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                if (nextGrid[x][y].active)
                {
                    olc::Pixel pix = olc::WHITE;
                    pDrawTarget->pColData[y * width + x] = pix;
                }

                // if the map changed (it may not have because it was paused)
                if(mapChanged)
                {
                    // the next frames pastGrid will be this frames nextGrid
                    pastGrid[x][y] = nextGrid[x][y];
                    // Clear nextGrid for the next frame
                    nextGrid[x][y] = Cell();
                }
            }
        }

        
        DrawOverlays();

        particleCount = 0;
        return true;
    }


    void UserInput()
    {

        auto mouse = GetMousePos();
        if (GetMouse(0).bHeld)
        {
            pastGrid[mouse.x][mouse.y].active = true;
        }

        if (GetMouse(1).bHeld)
        {
            pastGrid[mouse.x][mouse.y].active = false;
        }
        
        // Toggles the simulation
        if(GetKey(olc::SPACE).bPressed){
            simulate = !simulate;
        }

        // Steps the simulation
        if(GetKey(olc::F).bPressed)
        {
            simulateOnce = true;
        } 

        // Resets the simulation
        if(GetKey(olc::C).bPressed)
        {
            ResetSimulation();
        } 

        // Randomises the simulation
        if(GetKey(olc::R).bPressed)
        {
            Randomise(5);
        } 

    }

    void DrawOverlays()
    {
        // Not really the center as FillRect's center is the top left point
        auto center = olc::vi2d(3,3);
#define addcenter(x,y) center + olc::vi2d(x,y)
#define mulscreen(x,y) olc::vi2d(width * x, height * y)

        // Draws the current particle count
        DrawString(center + mulscreen(0, 0.12), std::to_string(particleCount), olc::WHITE);

        // Draws the current mouse position, useful for seeing world coords
        {
            auto mouse = GetMousePos();
            DrawString(center + mulscreen(0, 0.18),  "POS:" + std::to_string(mouse.x) + "," + std::to_string(mouse.y), olc::WHITE);
        }
    }

};

int main ()
{
    GameOfLife main;
    // main.Constructs first 2 parameters are width and height in pixels, however it's the width and height of each
    // PGE pixel. meaning 256 * 256, with a pixel size of 2 * 2, will actually display a window of 512 * 512
    if (main.Construct(main.width, main.height, MAPRATIO, MAPRATIO, false, false))
    {
        main.Start();
    }
    return 0;
}
