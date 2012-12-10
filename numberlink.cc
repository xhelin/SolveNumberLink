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

struct Box
{
  struct BoxIndex
  {
    BoxIndex(): x_(0), y_(0)
    {
    };
    BoxIndex(char x, char y): x_(x), y_(y)
    {
    };
    inline bool operator==(const BoxIndex& index)
    {
      return x_ == index.x_ && y_ == index.y_;
    }

    char x_;
    char y_;
  };

  struct VirtualColor
  {
    VirtualColor()
    {
      for (int i = 0; i < g_color_status.size(); ++i)
      {
        if (g_color_status[i])
        {
          this->possible_colors_.insert(i);
        };
      }
    };

    static void SetColorNum(int color_num)
    {
      g_color_num = color_num;
      g_color_status.reserve(color_num);
      g_color_status.push_back(0);
      for (int i = 0; i < color_num; ++i)
      {
        g_color_status.push_back(1);
      };
      return;
    };

    static void Merge(int vc_index1, int vc_index2)
    {
      if (vc_index1 == vc_index2)
        return;

      VirtualColor& vc1 = Box::g_virtualcolors[vc_index1];
      VirtualColor& vc2 = Box::g_virtualcolors[vc_index2];
      vc1.box_indexes_.insert(vc1.box_indexes_.end(), vc2.box_indexes_.begin(), vc2.box_indexes_.end());

      set<char> s;

      for (set<char>::iterator iter = vc1.possible_colors_.begin(); iter != vc1.possible_colors_.end(); ++iter)
      {
        if (vc2.possible_colors_.find(*iter) != vc2.possible_colors_.end())
        {
          s.insert(*iter);
        }
      }
      vc1.possible_colors_.swap(s);

      for (int i = 0; i < vc2.box_indexes_.size(); ++i)
      {
        Box& b = Box::Get(vc2.box_indexes_[i]);
        b.v_color_idx_ = vc_index1;
      }

      vc2.possible_colors_.clear();
      vc2.box_indexes_.clear();
    };

    void ChangeToRealColor(int c)
    {
      for (int i = 0; i < box_indexes_.size(); ++i)
      {
        Box& b = Box::Get(box_indexes_[i]);
        b.v_color_idx_ = -1;
        b.SetColor(c);
      };
      box_indexes_.clear();
    };

    bool TestChangeToRealColor()
    {
      int num = PossibleColorNum();
      if (num != 1)
        return false;
      ChangeToRealColor(*possible_colors_.begin());
      return true;
    };

    bool IsPossible(char color)
    {
      return possible_colors_.find(color) != possible_colors_.end();
    }

    void AddBox(const BoxIndex& b_index)
    {
      box_indexes_.push_back(b_index);
    };

    int PossibleColorNum()
    {
      for (int i = 1; i < g_color_status.size(); ++i)
      {
        if (!g_color_status[i])
        {
          possible_colors_.erase(i);
        }
      };
      return possible_colors_.size();
    };

    bool SetImpossibleColor(char color)
    {
      set<char>::iterator iter = possible_colors_.find(color);
      if (iter != possible_colors_.end())
      {
        possible_colors_.erase(iter);
        return true;
      }
      return false;
    }

    set<char> possible_colors_;
    vector<BoxIndex> box_indexes_;

    static vector<int> g_color_status;
    static int g_color_num;
  };

  static void SetHW(int height, int width)
  {
    g_height = height;
    g_width = width;
  }

  Box()
  {
  }

  Box(char x, char y, char clr):
  index_(x, y), is_end_(false), color_(clr), v_color_idx_(-1)
  {
    neighbors_.reserve(4);
    friends_.reserve(2);
    if (clr != 0)
      is_end_ = true;
    else {
      VirtualColor vc = VirtualColor();
      vc.AddBox(index_);
      Box::g_virtualcolors.push_back(vc);
      v_color_idx_ = Box::g_virtualcolors.size() - 1;
    }
  };

  int NeedFriendNum() const
  {
    if (is_end_)
      return 1 - friends_.size();
    else
      return 2 - friends_.size();
  }

  inline bool IsFriendFull() const
  {
    if (is_end_)
      return friends_.size() == 1;
    else
      return friends_.size() == 2;
  }

  inline bool HasColor() const
  {
    return color_ != 0;
  }

  inline bool IsEnd() const
  {
    return is_end_;
  }

  inline bool IsCorner() const
  {
    return neighbors_.size() == 2;
  }

  inline bool IsSide() const
  {
    return neighbors_.size() == 3;
  }

  inline bool IsInner() const
  {
    return neighbors_.size() == 4;
  }

  VirtualColor& GetVirtualColor()
  {
    if (v_color_idx_ < 0)
      throw "VirtualColor Index < 0";
    return g_virtualcolors[v_color_idx_];
  }

  void SetColor(int c)
  {
    color_ = c;

    for (int i = 0; i < this->neighbors_.size(); ++i)
    {
      Box& b = Box::Get(this->neighbors_[i]);
      if (b.color_ == c)
      {
        int already_friend = false;
        for (int i = 0; i < friends_.size(); ++i)
        {
          if (friends_[i] == b.index_)
          {
            already_friend = true;
            break;
          }
        }

        if (already_friend)
          continue;

        b.friends_.push_back(index_);
        friends_.push_back(b.index_);
      }
    }

    if (v_color_idx_ != -1)
    {
      g_virtualcolors[v_color_idx_].ChangeToRealColor(c);
    }

    v_color_idx_ = -1;
  }

  static bool MakeFriend(const BoxIndex& index1, const BoxIndex& index2)
  {
    Box& b1 = Get(index1);
    Box& b2 = Get(index2);
    for (int i = 0; i < b1.friends_.size(); ++i)
    {
      if (b1.friends_[i] == b2.index_)
      {
        return true;
      }
    }

    if (b1.HasColor() && b2.HasColor())
    {
      if (b1.color_ != b2.color_)
        return false;
      else
        return true;
    }

    b1.friends_.push_back(b2.index_);
    b2.friends_.push_back(b1.index_);

    if (b1.HasColor() && !b2.HasColor())
    {
      b2.SetColor(b1.color_);
    }
    else if (!b1.HasColor() && b2.HasColor())
    {
      b1.SetColor(b2.color_);
    }
    else
      VirtualColor::Merge(b1.v_color_idx_, b2.v_color_idx_);
    return true;
  }

  static Box& Get(int x, int y)
  {
    return g_boxes[y][x];
  }

  static Box& Get(const BoxIndex& index)
  {
    return g_boxes[index.y_][index.x_];
  }

  static bool IsOk()
  {
    for (int i = 0; i < Box::g_height; ++i)
    {
      for (int j = 0; j < Box::g_width; ++j)
      {
        Box& b = Get(j, i);
        if (!b.IsFriendFull() || !b.HasColor())
          return false;
      }
    }
    return true;
  }

  static bool MakeStable()
  {
    bool changed = true;

    while (changed)
    {
      changed = false;

      for (int i = 0; i < g_virtualcolors.size(); ++i)
      {
        if (g_virtualcolors[i].box_indexes_.empty())
          continue;
        if (g_virtualcolors[i].TestChangeToRealColor())
          changed = true;
        if (g_virtualcolors[i].possible_colors_.size() == 0)
        {
          //cout << __LINE__ << ": return false" << endl;
          return false;
        }
      }
      for (int i = 0; i < g_height; ++i)
      {
        for (int j = 0; j < g_width; ++j)
        {
          Box& current = Get(j, i);
          if (current.IsFriendFull())
            continue;

          vector<BoxIndex> availables;
          vector<BoxIndex> colored_boxes;
          bool all_same_color = true;
          vector<BoxIndex> uncolored_boxes;
          vector<BoxIndex> same_colored_boxes;
          availables.reserve(4);
          colored_boxes.reserve(4);

          for (int k = 0; k < current.neighbors_.size(); ++k)
          {
            /*
            bool already_friend = false;
            for (int m = 0; m < current.friends_.size(); ++m)
            {
              if (current.friends_[m] == current.neighbors_[m])
              {
                already_friend = true;
                break;
              }
            }
            if (already_friend)
              continue;
              */

            Box& temp = Get(current.neighbors_[k]);
            if (temp.IsFriendFull())
            {
              if (temp.HasColor() && !current.HasColor() && current.GetVirtualColor().SetImpossibleColor(temp.color_))
              {
                changed = true;
              }
              continue;
            }

            if (temp.HasColor() && current.HasColor() && temp.color_ != current.color_)
            {
              continue;
            }

            availables.push_back(temp.index_);
            if (!temp.HasColor())
            {
              uncolored_boxes.push_back(temp.index_);
              continue;
            }

            colored_boxes.push_back(temp.index_);

            if (temp.color_ != Get(colored_boxes[0]).color_)
              all_same_color = false;

            if (temp.color_ == current.color_)
              same_colored_boxes.push_back(temp.index_);
          }

          if ((int)availables.size() < current.NeedFriendNum())
          {
            //cout << __LINE__ << ": return false" << endl;
            return false;
          }

          if (current.IsEnd())
          {
            if (uncolored_boxes.empty())
            {
              //cout << __LINE__ << ": return false" << endl;
              return false;
            }
            else if(uncolored_boxes.size() == 1)
            {
              if (!MakeFriend(current.index_, uncolored_boxes[0]))
              {
                //cout << __LINE__ << ": return false" << endl;
                return false;
              }
              changed = true;
            }
          }
          else
          {
            if (current.HasColor())
            {
              if (same_colored_boxes.size() > 2)
              {
                //cout << __LINE__ << ": return false" << endl;
                return false;
              }

              for (int m = 0; m < same_colored_boxes.size(); ++m)
              {
                if (!MakeFriend(current.index_, same_colored_boxes[m]))
                {
                  //cout << __LINE__ << ": return false" << endl;
                  return false;
                }
              }

              if (current.IsFriendFull())
                continue;

              if (current.NeedFriendNum() == uncolored_boxes .size())
              {
                for (int m = 0; m < uncolored_boxes.size(); ++m)
                {
                  if (!MakeFriend(current.index_, uncolored_boxes[m]))
                  {
                    //cout << __LINE__ << ": return false" << endl;
                    return false;
                  }
                }
                changed = true;
              }
              else if (uncolored_boxes.size() < current.NeedFriendNum())
              {
                //cout << __LINE__ << ": return false. current = " << (int)current.index_.x_ << ":" << (int)current.index_.y_ << endl;
                return false;
              }
            }
            else
            {
              if (availables.size() == current.NeedFriendNum())
              {
                for (int m = 0; m < availables.size(); ++m)
                {
                  if (!MakeFriend(current.index_, availables[m]))
                  {
                    //cout << __LINE__ << ": return false" << endl;
                    return false;
                  }
                }
                changed = true;
              }
            }
          }

        }  // for (int j = 0; j < W; ++j)

      }  // for (int i = 0; i < H; ++i)

      for (int i = 0; i < g_height - 1; ++i)
      {
        for (int j = 0; j < g_width - 1; ++j)
        {
          vector<Box> vboxes;
          vboxes.push_back(Get(j, i));
          vboxes.push_back(Get(j+1, i));
          vboxes.push_back(Get(j, i+1));
          vboxes.push_back(Get(j+1, i+1));

          char vcolor = -1, color = 0;
          for (int k = 0; k < vboxes.size(); ++k)
          {
            if (vboxes[k].color_) {
              if (color == 0)
                color = vboxes[k].color_;
              else if (color != vboxes[k].color_) {
                color = 0;
                break;
              }
            }
            else if (vboxes[k].v_color_idx_ != -1) {
              if (vcolor == -1)
                vcolor = vboxes[k].v_color_idx_;
              else if (vcolor != vboxes[k].v_color_idx_) {
                vcolor = -1;
                break;
              }
            }
          }

          if (vcolor == -1 || color == 0)
            continue;

          if (g_virtualcolors[vcolor].SetImpossibleColor(color))
            changed = true;
        }
      }

    }  // while (changed)
    return true;
  }

  static ostream& PrintStat(ostream& o)
  {
    o << "Box:" << endl;
    for (int i = 0; i < g_boxes.size(); ++i)
    {
      vector<Box>& vboxes = g_boxes[i];
      for (int j = 0; j < vboxes.size(); ++j)
      {
        o << "\t";
        if (vboxes[j].color_)
          o << (int)vboxes[j].color_;
        else
          o << "(" << (int)vboxes[j].v_color_idx_ << ")";
      }
      o << "\n";
    }

    return o;
  }

  BoxIndex index_;
  bool is_end_;
  char color_;
  char v_color_idx_;

  vector<BoxIndex> neighbors_;
  vector<BoxIndex> friends_;


  // global variables

  static int g_width, g_height;
  static vector<vector<Box> > g_boxes;
  static vector<VirtualColor> g_virtualcolors;
};


int Box::g_width = 0;
int Box::g_height = 0;
int Box::VirtualColor::g_color_num = 0;
vector<int> Box::VirtualColor::g_color_status;
vector<vector<Box> > Box::g_boxes;
vector<Box::VirtualColor> Box::g_virtualcolors;



bool Search(const Box::BoxIndex& index)
{
  //cout << __LINE__ << endl;
  if (Box::MakeStable() == false)
  {
    //cout << __LINE__ << ": return false" << endl;
    return false;
  }
  //Box::PrintStat(cout);
  if (Box::IsOk())
  {
    //cout << __LINE__ << ": return true" << endl;
    return true;
  }

  for (int i = index.y_; i < Box::g_height; ++i)
  {
    for (int j = index.x_; j < Box::g_width; ++j)
    {
      if (Box::Get(j, i).HasColor())
        continue;
      else
      {
        //cout << "( " << i << " , " << j << " )" << endl;
        for (int k = 0; k < Box::VirtualColor::g_color_status.size(); ++k)
        {
          if (Box::VirtualColor::g_color_status[k])
          {
            Box& b = Box::Get(j, i);
            if (b.GetVirtualColor().IsPossible(Box::VirtualColor::g_color_status[k]))
            {
              //cout << "Guessing " << "( " << i << " , " << j << " )" << "color is " << k << endl;
              vector<vector<Box> > temp_boxes = Box::g_boxes;
              vector<Box::VirtualColor> temp_vc = Box::g_virtualcolors;
              vector<int> temp_color_status = Box::VirtualColor::g_color_status;

              b.GetVirtualColor().ChangeToRealColor(k);

              Box::BoxIndex next_index;
              if (j + 1 == Box::g_width)
              {
                next_index.x_= 0;
                next_index.y_ = i + 1;
              }
              else
              {
                next_index.x_ = j + 1;
                next_index.y_ = i;
              }

              if (Search(next_index))
              {
                //cout << __LINE__ << ": return true" << endl;
                return true;
              }
              else
              {
                //cout << "Guessing " << "( " << i << " , " << j << " )" << "color is " << k << " failed."<< endl;
              }
              Box::g_boxes = temp_boxes;
              Box::g_virtualcolors = temp_vc;
              Box::VirtualColor::g_color_status = temp_color_status;
            }
          }
        }
        break;
      }
    }
  }
  return false;
}


int main(int argc, char const *argv[])
{
  ifstream fin(argv[1]);
  ofstream fout("numberlink.out");

  int height, width, color_num;
  fin >> height >> width >> color_num;
  Box::SetHW(height, width);
  Box::VirtualColor::SetColorNum(color_num);

  Box::g_boxes.reserve(height);
  // read colors from file
  for (int i = 0; i < Box::g_height; ++i)
  {
    Box::g_boxes.push_back(vector<Box>());
    vector<Box>& boxes = Box::g_boxes.back();
    boxes.reserve(width);

    for (int j = 0; j < Box::g_width; ++j)
    {
      int color;
      fin >> color;
      boxes.push_back(Box(j, i, color));
    }
  }

  // set neighbors
  for (int i = 0; i < Box::g_height; ++i)
  {
    for (int j = 0; j < Box::g_width; ++j)
    {
      Box& b = Box::Get(j, i);
      if (i > 0)
        b.neighbors_.push_back(Box::BoxIndex(j, i-1));
      if (j > 0)
        b.neighbors_.push_back(Box::BoxIndex(j-1, i));
      if (i + 1 < Box::g_height)
        b.neighbors_.push_back(Box::BoxIndex(j, i+1));
      if (j + 1 < Box::g_width)
        b.neighbors_.push_back(Box::BoxIndex(j+1, i));
    }
  }

  Box::g_virtualcolors.reserve(Box::g_height * Box::g_width);

  // Box::MakeStable();
  // cout << "exit..." << endl;
  // Box::PrintStat(cout);

  Search(Box::BoxIndex());
  cout << "exit..." << endl;
  Box::PrintStat(cout);
  return 0;
}
