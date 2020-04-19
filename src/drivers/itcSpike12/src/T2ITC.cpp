#include "T2ITC.h"
#include "T2Tile.h"

#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>    // For close
#include <stdio.h>     // For snprintf
#include <errno.h>     // For errno

namespace MFM {
  void T2ITC::onTimeout(TimeQueue& srcTq) {
    pollPackets();
    insert(srcTq, 30, 3);
  }

  const char * T2ITC::getName() const {
    static char buf[30];
    snprintf(buf,30,"ITC/%s",mName);
    return buf;
  }

  void T2ITC::pollPackets() {
    while (tryHandlePacket()) { }
  }

  bool T2ITC::tryHandlePacket() {
    const u32 MAX_PACKET_SIZE = 256;
    u8 packet[MAX_PACKET_SIZE+1];
    int len = ::read(mFD, packet, MAX_PACKET_SIZE);
    if (len < 0) {
      if (errno == EAGAIN) return false;
      if (errno == ERESTART) return true; // try me again
    }
    if ((len == 1) && ((packet[0]&0xc0)==0xc0)) {
      u8 cmd = packet[0]&0x1f;
      const char * statusName;
      switch (cmd) {
      case 1: /*compatible */
        this->mCompatibilityStatus = 1;
        statusName = "COMPATIBLE";
        break;
      case 2: /*incompatible */
        this->mCompatibilityStatus = -1;
        statusName = "INCOMPATIBLE";
        break;
      default:/*wth*/
        this->mCompatibilityStatus = 0; 
        statusName = "ERROR/UNKNOWN";
        break;
      }
      LOG.Message("%s status %s", getName(), statusName);
      resetAllCircuits();  /*No matter what..*/
    } else {
      LOG.Warning("%s XXX HANDLE %d byte 0x%02x packet", getName(), len, packet[0]);
    }
    return true;
  }

  void T2ITC::resetAllCircuits() {
    LOG.Warning("%s XXX IMPLEMENT ME", __FUNCTION__);
  }

  T2ITC::T2ITC(T2Tile& tile, Dir6 dir6, const char * name)
    : mTile(tile)
    , mDir6(dir6)
    , mDir8(mapDir6ToDir8(dir6))
    , mName(name)
    , mActiveFreeCount(CIRCUIT_COUNT)
    , mFD(-1)
    , mCompatibilityStatus(0)
  {
    for (u8 i = 0; i < CIRCUIT_COUNT; ++i) {
      mActiveFree[i] = i;
      for (u8 act = 0; act <= 1; ++act) {
        mCircuits[act][i].mNumber = i;
        mCircuits[act][i].mEW = 0;  // Unassigned
      }
    }
  }

  CircuitNum T2ITC::tryAllocateActiveCircuit() {
    if (mActiveFreeCount==0) return ALL_CIRCUITS_BUSY;
    Random & r = mTile.getRandom();
    u8 idx = r.Between(0,mActiveFreeCount-1);
    CircuitNum ret = mActiveFree[idx];
    mActiveFree[idx] = mActiveFree[--mActiveFreeCount];
    return ret;
  }

  void T2ITC::freeActiveCircuit(CircuitNum cn) {
    assert(cn < CIRCUIT_COUNT);
    assert(mActiveFreeCount < CIRCUIT_COUNT);
    mActiveFree[mActiveFreeCount++] = cn;
    mCircuits[1][cn].mEW = 0;
  }

  const char * T2ITC::path() const {
    static char buf[100];
    snprintf(buf,100,"/dev/itc/mfm/%s",mName);
    return buf;
  }

  int T2ITC::open() {
    int ret = ::open(path(),O_RDWR|O_NONBLOCK);
    if (ret < 0) return -errno;
    mFD = ret;
    return ret;
  }

  int T2ITC::close() {
    int ret = ::close(mFD);
    mFD = -1;
    if (ret < 0) return -errno;
    return ret;
  }
}
