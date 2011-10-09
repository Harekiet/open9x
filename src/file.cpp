/*
 *
 * gruvin9x Author Bryan J.Rentoul (Gruvin) <gruvin@gmail.com>
 *
 * gruvin9x is based on code named er9x by
 * Author - Erez Raviv <erezraviv@gmail.com>, which is in turn
 * based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include "gruvin9x.h"
#include "stdio.h"
#include "inttypes.h"
#include "string.h"

uint8_t  s_write_err = 0;    // error reasons
#ifdef EEPROM_ASYNC_WRITE
uint8_t  s_sync_write = false;
#endif

#define RESV     64  //reserv for eeprom header with directory (eeFs)
#define FIRSTBLK (RESV/BS)
#define BLOCKS   (EESIZE/BS)

#define EEFS_VERS 4
struct DirEnt{
  uint8_t  startBlk;
  uint16_t size:12;
  uint16_t typ:4;
}__attribute__((packed));
#define MAXFILES (1+MAX_MODELS+3)
struct EeFs{
  uint8_t  version;
  uint8_t  mySize;
  uint8_t  freeList;
  uint8_t  bs;
  DirEnt   files[MAXFILES];
}__attribute__((packed)) eeFs;


static uint8_t EeFsRead(uint8_t blk,uint8_t ofs){
  uint8_t ret;
  eeprom_read_block(&ret,(const void*)(blk*BS+ofs),1);
  return ret;
}
static void EeFsWrite(uint8_t blk,uint8_t ofs,uint8_t val){
  eeWriteBlockCmp(&val, (void*)(blk*BS+ofs), 1);
}

static uint8_t EeFsGetLink(uint8_t blk){
  return EeFsRead( blk,0);
}
static void EeFsSetLink(uint8_t blk,uint8_t val){
  EeFsWrite( blk,0,val);
}
static uint8_t EeFsGetDat(uint8_t blk,uint8_t ofs){
  return EeFsRead( blk,ofs+1);
}
static void EeFsSetDat(uint8_t blk,uint8_t ofs,uint8_t*buf,uint8_t len){
  //EeFsWrite( blk,ofs+1,val);
  eeWriteBlockCmp(buf, (void*)(blk*BS+ofs+1), len);
}
static void EeFsFlushFreelist()
{
  eeWriteBlockCmp(&eeFs.freeList,&((EeFs*)0)->freeList ,sizeof(eeFs.freeList));
}
static void EeFsFlush()
{
  eeWriteBlockCmp(&eeFs, 0,sizeof(eeFs));
}

uint16_t EeFsGetFree()
{
  uint16_t  ret = 0;
  uint8_t i = eeFs.freeList;
  while( i ){
    ret += BS-1;
    i = EeFsGetLink(i);
  }
  return ret;
}

// #ifndef EEPROM_ASYNC_WRITE TODO because duplicate code
static void EeFsFree(uint8_t blk){///free one or more blocks
  uint8_t i = blk;
  while( EeFsGetLink(i)) i = EeFsGetLink(i);
  EeFsSetLink(i,eeFs.freeList);
  eeFs.freeList = blk; //chain in front
  EeFsFlushFreelist();
}
// #endif

#ifndef EEPROM_ASYNC_WRITE
static uint8_t EeFsAlloc(){ ///alloc one block from freelist
  uint8_t ret=eeFs.freeList;
  if(ret){
    eeFs.freeList = EeFsGetLink(ret);
    EeFsFlushFreelist();
    EeFsSetLink(ret,0);
  }
  return ret;
}
#endif

int8_t EeFsck()
{
#ifdef EEPROM_ASYNC_WRITE
  s_sync_write = true;
#endif

  uint8_t *bufp;
  static uint8_t buffer[BLOCKS];
  bufp = buffer;
  memset(bufp,0,BLOCKS);
  uint8_t blk ;
  int8_t ret=0;

  for(uint8_t i = 0; i <= MAXFILES; i++){
    uint8_t *startP = i==MAXFILES ? &eeFs.freeList : &eeFs.files[i].startBlk;
    uint8_t lastBlk = 0;
    blk = *startP;
    while(blk){
      if( (   blk <  FIRSTBLK ) //goto err_1; //bad blk index
          || (blk >= BLOCKS   ) //goto err_2; //bad blk index
          || (bufp[blk]       ))//goto err_3; //blk double usage
      {
        if(lastBlk){
          EeFsSetLink(lastBlk,0);
        }else{
          *startP = 0; //interrupt chain at startpos
          EeFsFlush();
        }
        blk=0; //abort
      }else{
        bufp[blk] = i+1;
        lastBlk   = blk;
        blk       = EeFsGetLink(blk);
      }
    }
  }
  for(blk = FIRSTBLK; blk < BLOCKS; blk++){
    if(bufp[blk]==0) {       //goto err_4; //unused block
      EeFsSetLink(blk,eeFs.freeList);
      eeFs.freeList = blk; //chain in front
      EeFsFlushFreelist();
    }
  }

#ifdef EEPROM_ASYNC_WRITE
  s_sync_write = false;
#endif
  return ret;
}

void EeFsFormat()
{
#ifdef EEPROM_ASYNC_WRITE
  s_sync_write = true;
#endif
  if(sizeof(eeFs) != RESV){
    extern void eeprom_RESV_mismatch();
    eeprom_RESV_mismatch();
  }
  memset(&eeFs,0, sizeof(eeFs));
  eeFs.version  = EEFS_VERS;
  eeFs.mySize   = sizeof(eeFs);
  eeFs.freeList = 0;
  eeFs.bs       = BS;
  for(uint8_t i = FIRSTBLK; i < BLOCKS; i++) EeFsSetLink(i,i+1);
  EeFsSetLink(BLOCKS-1, 0);
  eeFs.freeList = FIRSTBLK;
  EeFsFlush();
#ifdef EEPROM_ASYNC_WRITE
  s_sync_write = false;
#endif
}

bool EeFsOpen()
{
  eeprom_read_block(&eeFs,0,sizeof(eeFs));
#ifdef SIMU
  if(eeFs.version != EEFS_VERS)    printf("bad eeFs.version\n");
  if(eeFs.mySize  != sizeof(eeFs)) printf("bad eeFs.mySize\n");
#endif  

  return eeFs.version == EEFS_VERS && eeFs.mySize == sizeof(eeFs);
}

bool EFile::exists(uint8_t i_fileId)
{
  return eeFs.files[i_fileId].startBlk;
}

/*
 * swap two files in eeprom
 * called in sync AND async mode as it is a single write operation
 */
void EFile::swap(uint8_t i_fileId1, uint8_t i_fileId2)
{
  DirEnt            tmp = eeFs.files[i_fileId1];
  eeFs.files[i_fileId1] = eeFs.files[i_fileId2];
  eeFs.files[i_fileId2] = tmp;
  EeFsFlush();
}

void EFile::rm(uint8_t i_fileId)
{
#ifdef EEPROM_ASYNC_WRITE
  s_sync_write = true;
#endif
  uint8_t i = eeFs.files[i_fileId].startBlk;
  memset(&eeFs.files[i_fileId], 0, sizeof(eeFs.files[i_fileId]));
  EeFsFlush(); //chained out

  if(i) EeFsFree( i ); //chain in
#ifdef EEPROM_ASYNC_WRITE
  s_sync_write = false;
#endif
}

uint16_t EFile::size(){
  return eeFs.files[m_fileId].size;
}


// G: Open file ID for reading. Return the file's type
void EFile::openRd(uint8_t i_fileId){
  m_fileId = i_fileId;
  m_pos      = 0;
  m_currBlk  = eeFs.files[m_fileId].startBlk;
  m_ofs      = 0;
  s_write_err = ERR_NONE;       // error reasons */
}

void RlcFile::openRlc(uint8_t i_fileId)
{
  EFile::openRd(i_fileId);
  m_zeroes   = 0;
  m_bRlc     = 0;
}

uint8_t EFile::read(uint8_t*buf,uint16_t i_len){
  uint16_t len = eeFs.files[m_fileId].size - m_pos;
  if(len < i_len) i_len = len;
  len = i_len;
  while(len)
  {
    if(!m_currBlk) break;
    *buf++ = EeFsGetDat(m_currBlk, m_ofs++);
    if(m_ofs>=(BS-1)){
      m_ofs=0;
      m_currBlk=EeFsGetLink(m_currBlk);
    }
    len--;
  }
  m_pos += i_len - len;
  return i_len - len;
}

// G: Read runlength (RLE) compressed bytes into buf.
#ifdef TRANSLATIONS
uint16_t RlcFile::readRlc12(uint8_t*buf,uint16_t i_len, bool rlc2)
#else
uint16_t RlcFile::readRlc(uint8_t*buf,uint16_t i_len)
#endif
{
  uint16_t i=0;
  for( ; 1; ){
    uint8_t l=min<uint16_t>(m_zeroes,i_len-i);
    memset(&buf[i],0,l);
    i        += l;
    m_zeroes -= l;
    if(m_zeroes) break;

    l=min<uint16_t>(m_bRlc,i_len-i);
    uint8_t lr = read(&buf[i],l); 
    i        += lr ;
    m_bRlc   -= lr;
    if(m_bRlc) break;

    if(read(&m_bRlc,1)!=1) break; //read how many bytes to read

    assert(m_bRlc & 0x7f);
#ifdef TRANSLATIONS
    if (rlc2) {
#endif
      if(m_bRlc&0x80){ // if contains high byte
        m_zeroes  =(m_bRlc>>4) & 0x7;
        m_bRlc    = m_bRlc & 0x0f;
      }
      else if(m_bRlc&0x40){
        m_zeroes  = m_bRlc & 0x3f;
        m_bRlc    = 0;
      }
      //else   m_bRlc
#ifdef TRANSLATIONS
    }
    else {
      if(m_bRlc&0x80){ // if contains high byte
        m_zeroes  = m_bRlc & 0x7f;
        m_bRlc    = 0;
      }
    }
#endif
  }
  return i;
}

#ifdef EEPROM_ASYNC_WRITE

void RlcFile::write1(uint8_t b)
{
  m_write1_byte = b;
  write(&m_write1_byte, 1);
}

void RlcFile::write(uint8_t *buf, uint8_t i_len)
{
  m_write_len = i_len;
  m_write_buf = buf;

  do {
    nextWriteStep();
  } while (s_sync_write && m_write_len && !s_write_err);
}

void RlcFile::nextWriteStep()
{
  if(!m_currBlk && m_pos==0)
  {
    eeFs.files[FILE_TMP].startBlk = m_currBlk = eeFs.freeList;
    if (m_currBlk) {
      eeFs.freeList = EeFsGetLink(m_currBlk);
      m_write_step = WRITE_START_STEP + WRITE_FIRST_LINK;
      EeFsFlushFreelist();
      return;
    }
  }
  if (m_write_step == WRITE_START_STEP + WRITE_FIRST_LINK) {
    m_write_step = WRITE_START_STEP;
    EeFsSetLink(m_currBlk, 0);
    return;
  }

  while (m_write_len) {
    if (!m_currBlk) {
      s_write_err = ERR_FULL;
      break;
    }
    if (m_ofs >= (BS-1)) {
      m_ofs = 0;
      if (!EeFsGetLink(m_currBlk)) {
        if (!eeFs.freeList) {
          s_write_err = ERR_FULL;
          break;
        }
        m_write_step += WRITE_NEXT_LINK_1; // TODO review all these names
        EeFsSetLink(m_currBlk, eeFs.freeList);
        return;
      }
      m_currBlk = EeFsGetLink(m_currBlk);
    }
    switch (m_write_step & 0x0f) {
      case WRITE_NEXT_LINK_1:
        m_currBlk = eeFs.freeList;
        eeFs.freeList = EeFsGetLink(eeFs.freeList);
        m_write_step += 1;
        EeFsFlushFreelist();
        return;
      case WRITE_NEXT_LINK_2:
        m_write_step -= WRITE_NEXT_LINK_2;
        EeFsSetLink(m_currBlk, 0); // TODO needed?
        return;
    }
    if (!m_currBlk) { // TODO needed?
      s_write_err = ERR_FULL;
      break;
    }
    uint8_t tmp = BS-1-m_ofs; if(tmp>m_write_len) tmp=m_write_len;
    m_write_buf += tmp;
    m_write_len -= tmp;
    m_ofs += tmp;
    m_pos += tmp;
    EeFsSetDat(m_currBlk, m_ofs-tmp, m_write_buf-tmp, tmp);
    return;
  }

  if (s_write_err == ERR_FULL) {
    alert(PSTR("EEPROM overflow"));
    m_write_step = 0;
    m_write_len = 0;
  }

  if (!s_sync_write)
    nextRlcWriteStep();
}

#else

uint8_t RlcFile::write1(uint8_t b)
{
  return write(&b,1);
}

uint8_t RlcFile::write(uint8_t*buf, uint8_t i_len)
{
  uint8_t len=i_len;
  if(!m_currBlk && m_pos==0)
  {
    eeFs.files[m_fileId].startBlk = m_currBlk = EeFsAlloc();
  }
  while(len)
  {
#ifndef SIMU
    if( (int16_t)(m_stopTime10ms - get_tmr10ms()) < 0)
    {
      s_write_err = ERR_TMO;
      break;
    }
#endif
    if(!m_currBlk) {
      s_write_err = ERR_FULL;
      break;
    }
    if(m_ofs>=(BS-1)){
      m_ofs=0;
      if( ! EeFsGetLink(m_currBlk) ){
        EeFsSetLink(m_currBlk, EeFsAlloc());
      }
      m_currBlk = EeFsGetLink(m_currBlk);
    }
    if(!m_currBlk) {
      s_write_err = ERR_FULL;
      break;
    }
    uint8_t l = BS-1-m_ofs; if(l>len) l=len;
    EeFsSetDat(m_currBlk, m_ofs, buf, l);
    buf   +=l;
    m_ofs +=l;
    len   -=l;
  }
  m_pos += i_len - len;
  return   i_len - len;
}

#endif


#ifdef EEPROM_ASYNC_WRITE

void RlcFile::create(uint8_t i_fileId, uint8_t typ, uint8_t sync_write)
{
  // all write operations will be executed on FILE_TMP
  openRlc(FILE_TMP); // internal use
  eeFs.files[FILE_TMP].typ      = typ;
  eeFs.files[FILE_TMP].size     = 0;
  m_fileId = i_fileId;
  s_sync_write = sync_write;
}

void RlcFile::writeRlc(uint8_t i_fileId, uint8_t typ, uint8_t*buf, uint16_t i_len, uint8_t sync_write)
{
  create(i_fileId, typ, sync_write);

  m_write_step = WRITE_START_STEP;
  m_rlc_buf = buf;
  m_rlc_len = i_len;
  m_cur_rlc_len = 0;

  do {
    nextRlcWriteStep();
  } while (s_sync_write && m_write_step && !s_write_err);
}

void RlcFile::nextRlcWriteStep()
{
  uint8_t cnt    = 1;
  uint8_t cnt0   = 0;
  uint16_t i = 0;

  if (m_cur_rlc_len) {
    uint8_t tmp1 = m_cur_rlc_len;
    uint8_t *tmp2 = m_rlc_buf;
    m_rlc_buf += m_cur_rlc_len;
    m_cur_rlc_len = 0;
    write(tmp2, tmp1);
    return;
  }

  bool    run0   = m_rlc_buf[0] == 0;

  if(m_rlc_len==0) goto close;

  for (i=1; 1; i++) // !! laeuft ein byte zu weit !!
  {
    bool cur0 = m_rlc_buf[i] == 0;
    if (cur0 != run0 || cnt==0x3f || (cnt0 && cnt==0xf)|| i==m_rlc_len){
      if (run0) {
        assert(cnt0==0);
        if (cnt<8 && i!=m_rlc_len)
          cnt0 = cnt; //aufbew fuer spaeter
        else {
          m_rlc_buf+=cnt;
          m_rlc_len-=cnt;
          write1(cnt|0x40);
          return;
        }
      }
      else{
        m_rlc_buf+=cnt0;
        m_rlc_len-=cnt0+cnt;
        m_cur_rlc_len=cnt;
        if(cnt0){
          write1(0x80 | (cnt0<<4) | cnt);
        }
        else{
          write1(cnt);
        }
        return;
      }
      cnt=0;
      if (i==m_rlc_len) break;
      run0 = cur0;
    }
    cnt++;
  }

  close:

   switch(m_write_step) {
     case WRITE_START_STEP:
     {
       uint8_t fri=0;
       eeFs.files[FILE_TMP].size = m_pos;
       if (m_currBlk && ( fri = EeFsGetLink(m_currBlk))) {
         uint8_t prev_freeList = eeFs.freeList;
         eeFs.freeList = fri;
         while( EeFsGetLink(fri)) fri = EeFsGetLink(fri);
         m_write_step = WRITE_FREE_UNUSED_BLOCKS_STEP1;
         EeFsSetLink(fri, prev_freeList);
         return;
       }
     }

     case WRITE_FLUSH_STEP: // TODO could be avoided
       m_write_step = WRITE_SWAP_STEP;
       EeFsFlush(); //chained out
       return;

     case WRITE_FREE_UNUSED_BLOCKS_STEP1:
       m_write_step = WRITE_FREE_UNUSED_BLOCKS_STEP2;
       EeFsSetLink(m_currBlk, 0);
       return;

     case WRITE_FREE_UNUSED_BLOCKS_STEP2:
       m_write_step = WRITE_FLUSH_STEP;
       EeFsFlushFreelist();
       return;

     case WRITE_SWAP_STEP:
       m_write_step = 0;
       EFile::swap(m_fileId, FILE_TMP);
   }
}

void RlcFile::flush()
{
  while (eeprom_buffer_size > 0) wdt_reset();

  s_sync_write = true;
  while (m_write_len && !s_write_err)
    nextWriteStep();
  while (isWriting() && !s_write_err)
    nextRlcWriteStep();
  s_sync_write = false;
}

#else

void RlcFile::create(uint8_t i_fileId, uint8_t typ, uint16_t maxTme10ms)
{
  openRlc(i_fileId); //internal use
  eeFs.files[i_fileId].typ      = typ;
  eeFs.files[i_fileId].size     = 0;
  m_stopTime10ms = get_tmr10ms() + maxTme10ms;
}

// G: Write runlength (RLE) compressed bytes 
uint16_t RlcFile::writeRlc(uint8_t i_fileId, uint8_t typ,uint8_t*buf,uint16_t i_len, uint8_t maxTme10ms){

  create(i_fileId,typ,maxTme10ms);
  bool    run0   = buf[0] == 0;
  uint8_t cnt    = 1;
  uint8_t cnt0   = 0;
  uint16_t i     = 0;
  if(i_len==0) goto close;
  
  //RLE compression:
  //rb = read byte
  //if (rb | 0x80) write rb & 0x7F zeros
  //else write rb bytes
  for( i=1; 1 ; i++) // !! laeuft ein byte zu weit !!
  {
    bool cur0 = buf[i] == 0;
    if(cur0 != run0 || cnt==0x3f || (cnt0 && cnt==0xf)|| i==i_len){
      if(run0){
	assert(cnt0==0);
	if(cnt<8 && i!=i_len)
	  cnt0 = cnt; //aufbew fuer spaeter
	else {
	  if( write1(cnt|0x40)!=1)                goto error;//-cnt&0x3f
	}
      }else{
	if(cnt0){
	  if( write1(0x80 | (cnt0<<4) | cnt)!=1)  goto error;//-cnt0xx-cnt
	  cnt0 = 0;
	}else{
	  if( write1(cnt) !=1)                    goto error;//-cnt
	}
        uint8_t ret=write(&buf[i-cnt],cnt);
        if( ret !=cnt) { cnt-=ret;                goto error;}//-cnt
      }
      cnt=0;
      if(i==i_len) break;
      run0 = cur0;
    }
    cnt++;
  }
  if(0){
    error:
    i-=cnt+cnt0;
#ifdef SIMU
    switch(s_write_err){
      default:
      case ERR_NONE:
        assert(!"missing errno");
        break;  
      case ERR_FULL:
        printf("ERROR filesystem overflow! written: %d missing: %d\n",i,i_len-i);
        break;  
      case ERR_TMO:
        printf("ERROR filesystem write timeout %d 0ms\n",(int16_t)(m_stopTime10ms - g_tmr10ms));
        break;  
    }
#endif
  }
  close:
    close();
  return i;
}

#endif

// G: Close file and truncate at this blk. Add any remaining blocks to freeList chain
void RlcFile::close()
{
  uint8_t fri=0;

  if(m_currBlk && ( fri = EeFsGetLink(m_currBlk)))    EeFsSetLink(m_currBlk, 0);

#ifdef EEPROM_ASYNC_WRITE
  eeFs.files[FILE_TMP].size     = m_pos;
  EFile::swap(m_fileId, FILE_TMP);
#else
  eeFs.files[m_fileId].size     = m_pos;
  EeFsFlush(); //chained out
#endif

  if(fri) EeFsFree( fri );  //chain in

#ifdef EEPROM_ASYNC_WRITE
  m_write_step = 0;
  s_sync_write = false;
#endif
}


