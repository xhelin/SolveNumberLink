/**
solve number-link by program
author: Xiang Helin
email: xkeegan@gmail.com
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <deque>
#include <algorithm>
#include <map>
#include <set>
#include <tr1/unordered_map>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

struct Box;

struct TempColor
{
  void ChangeToRealColor(int c);
  void Merge(TempColor* tc);
  void AddBox(Box* b);

  set<char> impossible_colors;
  set<char> possible_colors;
  vector<Box*> boxes;
  static vector<TempColor> TempColors;
};

struct Box
{
  Box(): is_end(false), color(0), x(0), y(0), temp_color(0)
  {
    neighbors.reserve(4);
    friends.reserve(2);
  };

  int NeedFriendNum()
  {
    if (is_end)
      return 1 - friends.size();
    else
      return 2 - friends.size();    
  }
  
  inline bool IsFriendFull()
  {
    if (is_end)
      return friends.size() == 1;
    else
      return friends.size() == 2;
  }

  inline bool HasColor()
  {
    return color != 0;
  }

  inline bool IsEnd()
  {
    return is_end;
  }

  inline bool IsCorner()
  {
    return neighbors.size() == 2;
  }

  inline bool IsSide()
  {
    return neighbors.size() == 3;
  }

  inline bool IsInner()
  {
    return neighbors.size() == 4;
  }

  void SetColor(int c)
  {
    color = c;
    if (temp_color)
    {
      temp_color->ChangeToRealColor(c);
    }
  }

  bool MakeFriend(Box* b)
  {
    for (int i = 0; i < friends.size(); ++i)
    {
      if (friends[i] == b)
      {
        return true;
      }
    }

    if (color && b->color && b->color != color)
      return false;

    if (color && !b->color)
      b->SetColor(color);
    else if (!color && b->color)
      SetColor(b->color);
    else if (temp_color && b->temp_color)
      temp_color->Merge(b->temp_color);
    else if (temp_color && !b->temp_color)
    {
      temp_color->AddBox(b);
      b->temp_color = temp_color;
    }
    else if (!temp_color && b->temp_color)
    {
      b->temp_color->AddBox(b);
      temp_color = b->temp_color;
    }
    else
    {
      TempColor::TempColors.push_back(TempColor());
      TempColor* tc = &TempColor::TempColors[TempColor::TempColors.size()-1];
      tc->AddBox(this);
      tc->AddBox(b);
      temp_color = tc;
      b->temp_color = tc;
    }

    friends.push_back(b);
    b->friends.push_back(this);

    return true;
  }

  bool is_end;
  char color;
  char x, y;
  TempColor* temp_color;

  vector<Box*> neighbors;
  vector<Box*> friends;

};

// global definitions

Box** boxes;
int W, H;
int ColorNum = 0;

vector<TempColor> TempColor::TempColors;

bool MakeStable()
{
  bool changed = true;

  while (changed)
  {
    changed = false;

    for (int i = 0; i < H; ++i)
    {
      for (int j = 0; j < W; ++j)
      {
        Box& current = boxes[i][j]; 
        if (current.IsFriendFull())
          continue;

        vector<Box*> availables;
        vector<Box*> colored_boxes;
        bool all_same_color = true;
        vector<Box*> uncolored_boxes;
        vector<Box*> same_colored_boxes;
        availables.reserve(4);
        colored_boxes.reserve(4);

        for (int k = 0; k < current.neighbors.size(); ++k)
        {
          Box* temp = current.neighbors[k];
          if (temp->IsFriendFull())
            continue;

          availables.push_back(temp);
          if (!temp->HasColor())
          {
            uncolored_boxes.push_back(temp);
            continue;
          }

          colored_boxes.push_back(temp);

          if (temp->color != colored_boxes[0]->color)
            all_same_color = false;

          if (temp->color == current.color)
            same_colored_boxes.push_back(temp);
        }

        if (availables.size() < current.NeedFriendNum())
          return false;

        if (current.IsEnd())
        {
          if (uncolored_boxes.empty())
            return false;
          else if(uncolored_boxes.size() == 1)
          {
            current.MakeFriend(uncolored_boxes[0]);
            changed = true;
          }
        }
        else
        {
          if (current.HasColor())
          {
            if (same_colored_boxes.size() > 2)
              return false;

            for (int m = 0; m < same_colored_boxes.size(); ++m)
            {
              if (!current.MakeFriend(same_colored_boxes[m]))
              {
                return false;
              }
            }

            if (current.IsFriendFull())
              continue;

            if (current.NeedFriendNum() == uncolored_boxes.size())
            {
              for (int m = 0; m < uncolored_boxes.size(); ++m)
              {
                if (!current.MakeFriend(uncolored_boxes[i]))
                {
                  return false;
                }
              }
              changed = true;
            }
            else if (uncolored_boxes.size() < current.NeedFriendNum())
            {
              return false;
            }
          }
          else
          {
            if (availables.size() == current.NeedFriendNum())
            {
              for (int m = 0; m < availables.size(); ++m)
              {
                if (!current.MakeFriend(availables[m]))
                {
                  return false;
                }
              }
              changed = true;
            }
          }
        }

      }  // for (int j = 0; j < W; ++j)

    }  // for (int i = 0; i < H; ++i)

  }  // while (changed)
  return true;
}

int main(int argc, char const *argv[])
{
  ifstream fin("numberlink.in");
  ofstream fout("numberlink.out");
  fin >> H >> W;
  boxes = new Box*[H];

  // read colors from file
  for (int i = 0; i < H; ++i)
  {
    boxes[i] = new Box[W];
    for (int j = 0; j < W; ++j)
    {
      boxes[i][j].x = j;
      boxes[i][j].y = i;
      char color;
      fin >> color;
      boxes[i][j].color = color;

      if (color == 0)
      {
        boxes[i][j].is_end = false;
      } 
      else
      {
        boxes[i][j].is_end = true;

        if (color > ColorNum)
        {
          ColorNum = color;
        }
      }
    }
  }

  // set neighbors
  for (int i = 0; i < H; ++i)
  {
    for (int j = 0; j < W; ++j)
    {
      if (i > 1)
        boxes[i][j].neighbors.push_back(&boxes[i-1][j]);
      if (j > 1)
        boxes[i][j].neighbors.push_back(&boxes[i][j-1]);
      if (i + 1 < H)
        boxes[i][j].neighbors.push_back(&boxes[i+1][j]);
      if (j + 1 < W)
        boxes[i][j].neighbors.push_back(&boxes[i][j+1]);
    }
  }


  TempColor::TempColors.reserve(W*H);
  MakeStable();

  return 0;
}