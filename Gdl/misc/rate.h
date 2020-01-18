
#ifndef _GdlRate_
#define _GdlRate_

class Rate {
  public:
  u32 last;
  u32 acc;
  u32 ticks;
  u32 px;

  Rate( u32 px );
  Rate( void );
  void reset( void );
  void ticksByPx( u32 ticks );
  void pxBySecond( u32 px );
  u32 update(void);
};

#endif
