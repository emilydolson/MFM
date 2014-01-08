#include "eucliddir.h"
#include "grid.h"

template <class T,u32 R>
Grid<T,R>::Grid(int width, int height, ElementTable<T,R>* elementTable)
{
  m_width = width;
  m_height = height;

  m_tiles = new Tile<T,R>[m_width * m_height];

  m_elementTable = elementTable;
}

template <class T,u32 R>
void Grid<T,R>::SetStateFunc(u32 (*stateFunc)(T* atom))
{
  for(u32 i = 0; i < m_width * m_height; i++)
  {
    m_tiles[i].SetStateFunc(stateFunc);
  }
}

template <class T, u32 R>
Grid<T,R>::~Grid()
{
  delete[] m_tiles;
}

template <class T, u32 R>
u32 Grid<T,R>::GetHeight()
{
  return m_height;
}

template <class T, u32 R>
u32 Grid<T,R>::GetWidth()
{
  return m_width;
}

template <class T, u32 R>
void Grid<T,R>::PlaceAtom(T& atom, Point<int>& loc)
{
  u32 x = loc.GetX() / TILE_WIDTH;
  u32 y = loc.GetY() / TILE_WIDTH;

  Point<int> local(loc.GetX() % TILE_WIDTH,
		   loc.GetY() % TILE_WIDTH);

  GetTile(x,y).PlaceAtom(atom, local);
}

template <class T, u32 R>
T* Grid<T,R>::GetAtom(Point<int>& loc)
{
  u32 x = loc.GetX() / TILE_WIDTH;
  u32 y = loc.GetY() / TILE_WIDTH;

  Point<int> local(loc.GetX() % TILE_WIDTH,
		   loc.GetY() % TILE_WIDTH);

  return GetTile(x,y).GetAtom(&local);
}

template <class T, u32 R>
void Grid<T,R>::Expand(int extraW, int extraH)
{
  Resize(m_width + extraW, m_height + extraH);
}

template <class T, u32 R>
void Grid<T,R>::Resize(int newWidth, int newHeight)
{
  delete m_tiles;

  m_width = newWidth;
  m_height = newHeight;

  m_tiles = new Tile<T,R>[m_width * m_height];
}

template <class T, u32 R>
void Grid<T,R>::TriggerEvent()
{
  Point<int> windowTile(true, m_width, m_height);

  m_tiles[windowTile.GetX() + 
	  windowTile.GetY() * m_width].Execute(*m_elementTable);

  m_lastEventTile.Set(windowTile.GetX(), windowTile.GetY());
}

template <class T, u32 R>
void Grid<T,R>::FillNeighbors(int center_x, int center_y,
			    Tile<T,R>** out)
{  
  for(int i = 0; i < 8; i++)
  {
    
  }
}

template <class T, u32 R>
void Grid<T,R>::FillLastEventTile(Point<int>& out)
{
  out.Set(m_lastEventTile.GetX(),
	  m_lastEventTile.GetY());
}
