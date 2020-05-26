#include <SDL.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <cmath>
#include <time.h>

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Renderer* renderer = NULL;

struct ColourRect
{
    SDL_Rect rect;
    Uint8 r;
    Uint8 g;
    Uint8 b;
};

struct Door
{
    int x;
    int y;
    Door* twin;
};

enum SplitType
{
    ROOT, TOP, BOTTOM, LEFT, RIGHT, HORIZONTAL, VERTICAL
};

bool init()
{
    bool success = true;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialise. SDL_Error: %s/n", SDL_GetError());
        success = false;
    }
    else
    {
        window = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if(renderer == NULL)
            {
                printf("renderer could not be created!/n");
                success = false;
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
    }
    return success;
}

void close()
{
    SDL_DestroyWindow(window);
    window = NULL;
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    SDL_FreeSurface(screenSurface);
    screenSurface = NULL;

    SDL_Quit();
}

class Node
{
    public:
    SDL_Rect* rect;
    SDL_Rect* room;
    Node*left;
    Node*right;
    SplitType split;

    Node(SDL_Rect* rect, SplitType splitType = ROOT)
    {
        this->rect = rect;
        this->left = nullptr;
        this->right = nullptr;
        this->room = nullptr;
        this->split = splitType;
    }

    void getRects(Node* n, vector<ColourRect*>* tmpVec)
    {
        if(n->left != nullptr)
        {
          getRects(n->left, tmpVec);
          getRects(n->right, tmpVec);
        }
        else
        {
            ColourRect* tempRect = new ColourRect{*(n->rect), Uint8(rand() % 255), Uint8(rand() % 255), Uint8(rand() % 255)};
            tmpVec->push_back(tempRect);
        }
        return;
    }

    void buildRoom(Node* root, vector<SDL_Rect*>* roomList)
    {
        SDL_Rect* tmpRoom = nullptr;
        if(root->left == nullptr)
        {
            int width = (rand() % (root->rect->w - 2)) + 2;
            int height = (rand() % (root->rect->h - 2)) + 2;
            int leftPad = rand() % (root->rect->w - width);
            int topPad = rand() % (root->rect->h - height);

            int x = root->rect->x + leftPad;
            int y = root->rect->y + topPad;

            tmpRoom = new SDL_Rect{x, y, width, height};
            root->room = tmpRoom;
            roomList->push_back(root->room);
        }
        else
        {
            buildRoom(root->left, roomList);
            buildRoom(root->right, roomList);
        }
    }

    Door* makeDoor(Node* n, vector<Door*>* hallList, SplitType split = ROOT)
    {
        Door* doorXY;
        switch(split)
        {
            case LEFT:
                doorXY = new Door();
                doorXY->x = (n->room->w - 1) + n->room->x;
                doorXY->y = (rand() % (n->room->h - 1)) + n->room->y;
                hallList->push_back(doorXY);
            return (*hallList)[hallList->size() - 1];
            case RIGHT:
                doorXY = new Door();
                doorXY->x = n->room->x;
                doorXY->y = (rand() % (n->room->h - 1 )) + n->room->y;
                hallList->push_back(doorXY);
                return (*hallList)[hallList->size() - 1];
            case TOP:
                doorXY = new Door();
                doorXY->x =(rand() % (n->room->w - 1)) + n->room->x;
                doorXY->y = n->room->y + (n->room->h - 1);
                hallList->push_back(doorXY);
                return (*hallList)[hallList->size() - 1];
            case BOTTOM:
                doorXY = new Door();
                doorXY->x = (rand() % (n->room->w - 1)) + n->room->x;
                doorXY->y = n->room->y;
                hallList->push_back(doorXY);
                return (*hallList)[hallList->size() - 1];
            case ROOT:
            case HORIZONTAL:
            case VERTICAL:
                return nullptr;
        }
        return nullptr;
    }


    Node* linkRooms(Node* root, vector<Door*>* hallList, SplitType branchSplit = ROOT)
    {
        Node* lSplitRoom = nullptr;
        Node* rSplitRoom = nullptr;
        Door* tmpLdoor = nullptr;
        Door* tmpRdoor = nullptr;
        switch(root->split)
        {
            case LEFT:
                return root;
            case RIGHT:
                return root;
            case TOP:
                return root;
            case BOTTOM:
                return root;
            case HORIZONTAL:

                lSplitRoom = linkRooms(root->left, hallList, LEFT);
                rSplitRoom = linkRooms(root->right, hallList, RIGHT);

                if(lSplitRoom == nullptr || rSplitRoom == nullptr)
                {
                    break;
                }
                tmpLdoor = makeDoor(lSplitRoom, hallList, LEFT);
                tmpRdoor = makeDoor(rSplitRoom, hallList, RIGHT);
                tmpLdoor->twin = tmpRdoor;
                tmpRdoor->twin = tmpLdoor;
                if(branchSplit == LEFT || branchSplit == TOP)
                {
                    return rSplitRoom;
                }
                else if(branchSplit == RIGHT || branchSplit == BOTTOM)
                {
                    return lSplitRoom;
                }
                else
                {
                    return nullptr;
                }
            case VERTICAL:

                lSplitRoom = linkRooms(root->left, hallList, TOP);
                rSplitRoom = linkRooms(root->right, hallList, BOTTOM);

                if(lSplitRoom == nullptr || rSplitRoom == nullptr)
                {
                    break;
                }

                tmpLdoor = makeDoor(lSplitRoom, hallList, TOP);
                tmpRdoor = makeDoor(rSplitRoom, hallList, BOTTOM);

                tmpLdoor->twin = tmpRdoor;
                tmpRdoor->twin = tmpLdoor;

                if(branchSplit == LEFT || branchSplit == TOP)
                {
                    return rSplitRoom;
                }
                else if(branchSplit == RIGHT || branchSplit == BOTTOM)
                {
                    return lSplitRoom;
                }
                else
                {
                    return nullptr;
                }
            case ROOT:
                return nullptr;
            default:
                return nullptr;
        }
        return nullptr;
    }
};

bool Vsplit(Node* n)
{
    if(n->rect->h > 7)
    {
        int splitPoint = 0;
        if(n->rect->h == 8)
        {
            splitPoint = 4;
        }
        else
        {
            splitPoint = (rand() % (n->rect->h - 8)) + 4;
        }

        SDL_Rect* rectT = new SDL_Rect{n->rect->x, n->rect->y, n->rect->w, splitPoint};
        SDL_Rect* rectB = new SDL_Rect{n->rect->x, n->rect->y + splitPoint, n->rect->w, n->rect->h - splitPoint};
        n->left = new Node(rectT, TOP);
        n->right = new Node(rectB, BOTTOM);
        n->split = VERTICAL;
        return true;
    }
    return false;
}

bool Hsplit(Node* n)
{
    if(n->rect->w > 7)
    {
        int splitPoint = 0;
        if(n->rect->w == 8)
        {
            splitPoint = 4;
        }
        else
        {
            splitPoint = (rand() % (n->rect->w - 8)) + 4;
        }

        SDL_Rect* rectL = new SDL_Rect{n->rect->x, n->rect->y, splitPoint, n->rect->h};
        SDL_Rect* rectR = new SDL_Rect{n->rect->x + splitPoint, n->rect->y, n->rect->w - splitPoint, n->rect->h};
        n->left = new Node(rectL, LEFT);
        n->right = new Node(rectR, RIGHT);
        n->split = HORIZONTAL;
        return true;
    }
    return false;
}

void splitNode(Node* n, int vert = -1, int percentToSplit = 75)
{
    if(vert == -1)
    {
        vert = (rand() % 2);
    }
    if((rand() % 100) > percentToSplit && n->split != ROOT)
    {
        return;
    }
    if(vert == 1)    //split vertically
    {
        if(Vsplit(n))
        {
            //split done
        }
        else
        {
            Hsplit(n);  //try hsplit instead
        }
    }
    else        //split horizontally
    {
        if(Hsplit(n))
        {
            //hsplit
        }
        else
        {
            Vsplit(n);  //try vsplit instead
        }
    }
    if(n->left != nullptr)
    {
        splitNode(n->left);
    }
    if(n->right != nullptr)
    {
        splitNode(n->right);
    }
    return;
};

void connectDoors(vector<Door*>* doorList, vector<vector<int>*>* levelMap)
{
    for(int i = 0; i < (int(doorList->size()) - 1); i++)
    {
        Door* doorA;
        Door* doorB;
        int xDif, yDif;
        int startX, startY, endX, endY;

        doorA = (*doorList)[i];
        doorB = doorA->twin;

        startX = doorA->x;
        startY = doorA->y;
        endX = doorB->x;
        endY = doorB->y;
        xDif = abs(endX - startX);
        yDif = abs(endY - startY);

        int x, y;
        int stepCount = 0;
        if((startX < endX) && (startY < endY))
        {
            if(xDif > yDif)
            {
                y = startY;
                for(x = startX; x <= endX; x++)
                {
                    (*(*levelMap)[x])[y] = 1;
                    stepCount += yDif;
                    if(stepCount >= xDif)
                    {
                        y++;
                        stepCount -= xDif;
                        (*(*levelMap)[x])[y] = 1;
                    }
                }
            }
            else
            {
                x = startX;
                for(y = startY; y <= endY; y++)
                {
                    (*(*levelMap)[x])[y] = 1;
                    stepCount += xDif;
                    if(stepCount >= yDif)
                    {
                        x++;
                        stepCount -= yDif;
                        (*(*levelMap)[x])[y] = 1;
                    }
                }
            }
        }
        else if((startX > endX) && (startY < endY))
        {
            if(xDif > yDif)
            {
                y = startY;
                for(x = startX; x >= endX; x--)
                {
                    (*(*levelMap)[x])[y] = 1;
                    stepCount += yDif;
                    if(stepCount >= xDif)
                    {
                        y++;
                        stepCount -= xDif;
                        (*(*levelMap)[x])[y] = 1;
                    }
                }
            }
            else
            {
                x = startX;
                for(y = startY; y <= endY; y++)
                {
                    (*(*levelMap)[x])[y] = 1;
                    stepCount += xDif;
                    if(stepCount >= yDif)
                    {
                        x--;
                        stepCount -= yDif;
                        (*(*levelMap)[x])[y] = 1;
                    }
                }
            }
        }
        else if((startX < endX) && (startY > endY))
        {
            if(xDif > yDif)
            {
                y = startY;
                for(x = startX; x <= endX; x++)
                {
                    (*(*levelMap)[x])[y] = 1;
                    stepCount += yDif;
                    if(stepCount >= xDif)
                    {
                        y--;
                        stepCount -= xDif;
                        (*(*levelMap)[x])[y] = 1;
                    }
                }
            }
            else
            {
                x = startX;
                for(y = startY; y >= endY; y--)
                {
                    (*(*levelMap)[x])[y] = 1;
                    stepCount += xDif;
                    if(stepCount >= yDif)
                    {
                        x++;
                        stepCount -= yDif;
                        (*(*levelMap)[x])[y] = 1;
                    }
                }
            }
        }
        else if((startX > endX) && (startY > endY))
        {
            if(xDif > yDif)
            {
                y = startY;
                for(x = startX; x >= endX; x--)
                {
                    (*(*levelMap)[x])[y] = 1;
                    stepCount += yDif;
                    if(stepCount >= xDif)
                    {
                        y--;
                        stepCount -= xDif;
                        (*(*levelMap)[x])[y] = 1;
                    }
                }
            }
            else
            {
                x = startX;
                for(y = startY; y >= endY; y--)
                {
                    (*(*levelMap)[x])[y] = 1;
                    stepCount += xDif;
                    if(stepCount >= yDif)
                    {
                        x--;
                        stepCount -= yDif;
                        (*(*levelMap)[x])[y] = 1;
                    }
                }
            }
        }
        else if(startX == endX)
        {
            x = startX;
            if(startY < endY)
            {
                for(y = startY; y <= endY; y++)
                {
                    (*(*levelMap)[x])[y] = 1;
                }
            }
            else
            {
                for(y = startY; y >= endY; y--)
                {
                    (*(*levelMap)[x])[y] = 1;
                }
            }
        }
        else if(startY == endY)
        {
            y = startY;
            if(startX < endX)
            {
                for(x = startX; x <= endX; x++)
                {
                    (*(*levelMap)[x])[y] = 1;
                }
            }
            else
            {
                for(x = startX; x >= endX; x--)
                {
                    (*(*levelMap)[x])[y] = 1;
                }
            }
        }
    }
    return;
};

void clearLevel(vector<vector<int>*>* L, int width, int height)
{
    for(int x = 0; x < 32; x++)
    {
        for(int y = 0; y < 24; y++)
        {
            /*if(x == 0 || x == (width - 1) || y == 0 || y == (height - 1))
            {
                (*(*L)[x])[y] = 0;
            }
            else
            {
                (*(*L)[x])[y] = 1;
            }*/
            (*(*L)[x])[y] = 0;
        }
    }
}

int main(int argc, char* args[])
{
    srand (time(NULL));
    bool quit = false;
    SDL_Event e;

    vector<vector<int>*> level;
    int levelWidth = 32;
    int levelHeight = 24;
    vector<int>* tVec;
    for(int i = 0; i < levelWidth; i++)
    {
        tVec = new vector<int>();
        for(int j = 0; j < levelHeight; j++)
        {
            int val = 1;
            tVec->push_back(val);
        }
        level.push_back(tVec);
    }
    clearLevel(&level, levelWidth, levelHeight);

    SDL_Rect levelRect = {1, 1, 30, 22};
    Node* bsp = new Node(&levelRect);

    splitNode(bsp);
    vector<ColourRect*> rectList;
    bsp->getRects(bsp, &rectList);

    vector<SDL_Rect*> roomList;
    bsp->buildRoom(bsp, &roomList);

    vector<Door*> hallList;
    if(bsp->linkRooms(bsp, &hallList) == nullptr)
    {
        //printf("Fine.\n");
    }
    else
    {
        printf("Not fine.\n");
    }

    connectDoors(&hallList, &level);

    if(!init())
    {
        printf("failed to initialise.\n");
    }
    else
    {
        //load media
        while(!quit)
        {
            while(SDL_PollEvent(&e) != 0)
            {
                if(e.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if(e.type == SDL_KEYDOWN)
                {
                    switch(e.key.keysym.sym)
                    {
                        case SDLK_SPACE:
                            clearLevel(&level, levelWidth, levelHeight);
                            bsp = new Node(&levelRect);
                            splitNode(bsp);
                            rectList.clear();
                            roomList.clear();
                            hallList.clear();
                            bsp->getRects(bsp, &rectList);
                            bsp->buildRoom(bsp, &roomList);
                            bsp->linkRooms(bsp, &hallList);
                            connectDoors(&hallList, &level);
                    }
                }
            }


            SDL_SetRenderDrawColor(renderer, 0xFF , 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(renderer);

            //draw grid

            /*for(int i = 0; i < int(rectList.size()); i++)
            {
                SDL_SetRenderDrawColor(renderer, rectList[i]->r, rectList[i]->g, rectList[i]->b, 0xFF);
                SDL_Rect tmpRect = rectList[i]->rect;
                SDL_Rect drawRect = {tmpRect.x * 20, tmpRect.y * 20, tmpRect.w * 20, tmpRect.h * 20};
                SDL_RenderFillRect(renderer, &drawRect);
            }
*/
            for(int j = 0; j < levelHeight; j++)
            {
                for(int i = 0; i < levelWidth; i++)
                {
                    vector<int> lvlX = (*level[i]);
                    int value = lvlX[j];
                    if(value == 1)
                    {
                        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    }
                    else if(value == 0)
                    {
                        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
                    }
                    SDL_Rect grid = {i*20, j*20, 20, 20};
                    SDL_RenderFillRect(renderer, &grid);
                }
            }

            for(int i = 0; i < int(roomList.size()); i++)
            {
                SDL_SetRenderDrawColor(renderer, 0x88, 0x88, 0xFF, 0xFF);
                SDL_Rect tmpRect = *(roomList[i]);
                SDL_Rect drawRect = {tmpRect.x * 20, tmpRect.y * 20, tmpRect.w * 20, tmpRect.h * 20};
                SDL_RenderFillRect(renderer, &drawRect);
            }

            for(int i = 0; i < int(hallList.size()); i++)
            {
                int x = hallList[i]->x;
                int y = hallList[i]->y;
                SDL_Rect tmpRect = { x * 20, y * 20, 20, 20 };
                SDL_RenderFillRect(renderer, &tmpRect);
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x44, 0xFF);
                //SDL_RenderDrawLine(renderer, x * 20 + 10, y * 20 + 10, hallList[i]->twin->x * 20 + 10, hallList[i]->twin->y * 20 + 10);
            }

            for(int i = 0; i < 32; i++)
            {
                for(int j = 0; j < 24; j++)
                {
                    SDL_Rect grid = {i*20, j*20, 20, 20};
                    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
                    SDL_RenderDrawRect(renderer, &grid);
                }
            }

            SDL_RenderPresent(renderer);
        }
    }

    close();

    return 0;
};

