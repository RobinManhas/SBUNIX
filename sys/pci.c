#include <sys/kprintf.h>
#include <sys/util.h>
#include <sys/pci.h>
#include <sys/defs.h>
#include <sys/ahci.h>
#include <sys/common.h>


// Reference: OSDev

#define SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define ADDRESS_SPACE_1GB 0x40000000 // hex value for 1 gb
#define ADDRESS_SPACE_REMAP 0xA6000 // remap below 1 gb
#define BUFFER_SPACE 0x300000
#define CONFIG_DATA 0xCFC
#define CONFIG_ADDRESS 0xCF8
#define AHCI_DEV_NULL 0
#define	SATA_SIG_PM	0x96690101	// Port multiplier
#define HBA_PORT_DET_PRESENT 3
#define HBA_PORT_IPM_ACTIVE 1

#define	AHCI_BASE	0x400000	// 4M

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

hba_mem_t *abar;
hba_port_t  *sataDrivePort;

// Find a free command list slot
int find_cmdslot(hba_port_t *port)
{
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact | port->ci);

    int cmdslots = ((abar->cap)>> 8)&(0x1F);
    //kprintf("slots  %d cmdslots count %d\n",slots,cmdslots);
    //kprintf("cmdslots count %d\n",cmdslots);
    for (int i=0; i< cmdslots; i++)
    {
        if ((slots&1) == 0)
            return i;
        slots >>= 1;
    }
    //kprintf("Cannot find free command list entry\n");
    return -1;
}

/*int executeCmd(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint32_t *buf, int isWrite)
{
    port->is_rwc = (uint32_t)-1;		// Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1) {
        kprintf("no slot found\n");
        return -1;
    }

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);	// Command FIS size
    cmdheader->w = isWrite&1;
    kprintf("header : %d", cmdheader->w);
    cmdheader->prdtl = (uint32_t)((count-1)>>4) + 1;	// PRDT entries count

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);


    // 8K bytes (16 sectors) per PRDT
    int i = 0;
    for (i=0; i<cmdheader->prdtl-1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8*1024;	// 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4*1024;	// 4K words
        count -= 16;	// 16 sector
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
    cmdtbl->prdt_entry[i].dbc = count<<9;	// 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;	// Command
    cmdfis->command = isWrite?0x35:0x25; //ATA_CMD_READ_DMA_EX;

    cmdfis->lba0 = (uint32_t)startl;
    cmdfis->lba1 = (uint32_t)(startl>>8);
    cmdfis->lba2 = (uint32_t)(startl>>16);
    cmdfis->device = 1<<6;	// LBA mode

    cmdfis->lba3 = (uint32_t)(startl>>24);
    cmdfis->lba4 = (uint32_t)starth;
    cmdfis->lba5 = (uint32_t)(starth>>8);

    cmdfis->count = count;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        kprintf("Port is hung\n");
        return -1;
    }

    port->ci = 1<<slot;	// Issue command

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0)
            break;
        if (port->is_rwc & HBA_PxIS_TFES)	// Task file error
        {
            kprintf("Read disk error1\n");
            return -1;
        }
    }

    // Check again
    if (port->is_rwc & HBA_PxIS_TFES)
    {
        kprintf("Read disk error2\n");
        return -1;
    }

    return 1;
}
*/

int executeCmd(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf, int isWrite)
{
    //int result = executeCmd(port,startl, starth, count, buf, 1 );
    //return result;
    port->is_rwc = (uint32_t)-1;		// Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1) {
        kprintf("no slot found\n");
        return -1;
    }

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    //kprintf("cmdheader c value %p\n",abar->cap);
    cmdheader += slot;
    cmdheader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);	// Command FIS size
    cmdheader->w = isWrite & 1;		// write to device
    cmdheader->c = 1;
    cmdheader->prdtl = (uint32_t)((count-1)>>4) + 1;	// PRDT entries count

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
    int i = 0;

    for ( i = 0; i < sizeof(hba_cmd_tbl_t)
                     + (cmdheader->prdtl-1) * sizeof(hba_prdt_entry_t); i ++ ) {
        *(((uint32_t *)cmdtbl) + i) = 0;
    }

    // 8K bytes (16 sectors) per PRDT
    for (i=0; i<cmdheader->prdtl-1; i++)
    {
        kprintf("inside write %x -> %d\n",buf,*buf );

        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 4*1024-1;	// 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4*1024;	// 4K words
        count -= 16;	// 16 sector
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
    cmdtbl->prdt_entry[i].dbc = count<<9;	// 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;	// Command
    cmdfis->command = isWrite?0x35:0x25; //ATA_CMD_WRITE_DMA_EX;

    cmdfis->lba0 = (uint32_t)startl;
    cmdfis->lba1 = (uint32_t)(startl>>8);
    cmdfis->lba2 = (uint32_t)(startl>>16);
    cmdfis->device = 1<<6;	// LBA mode

    cmdfis->lba3 = (uint32_t)(startl>>24);
    cmdfis->lba4 = (uint32_t)starth;
    cmdfis->lba5 = (uint32_t)(starth>>8);


    cmdfis->count = count;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    //kprintf("tfd value %x\n",port->tfd);
    if (spin == 1000000)
    {
        kprintf("Port is hung, tfd value %x\n",port->tfd);
        return -1;
    }

    port->ci = 1<<slot;	// Issue command


    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0)
            break;
        if (port->is_rwc & HBA_PxIS_TFES)	// Task file error
        {
            kprintf("write disk error\n");
            return -1;
        }
    }
    //kprintf("return complete");
    // Check again
    if (port->is_rwc & HBA_PxIS_TFES)
    {
        kprintf("write disk error\n");
        return -1;
    }

    return 1;

}

int read(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf) {
    return executeCmd(port,startl, starth, count, buf, 0 );
}

int write(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf) {
    return executeCmd(port,startl, starth, count, buf, 1 );
}



// Start command engine
void start_cmd(hba_port_t *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;

    //Code for hardware
    /*port->cmd = (port->cmd)|0x10000008;
    while(1){
        if((port->ssts & 0x0F)>1){
            if((port->cmd & 0x8)==0){
                break;
            }
        }
    }
    kprintf("Waiting for bsy %x\n",port->tfd);
    while(1){
        if((port->tfd &(ATA_DEV_BUSY|ATA_DEV_DRQ))==0){
            break;
        }
    }*/
    port->cmd |= HBA_PxCMD_ST;
}
// Stop command engine
void stop_cmd(hba_port_t *port)
{
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Wait until FR (bit14), CR (bit15) are cleared
    while(1)
    {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }

    port->cmd &= ~HBA_PxCMD_FRE;
    //Code for hardware
   /* unsigned int sctl = port->sctl;
    sctl = port->sctl;
    kprintf("Value of sctl %p\n",port->sctl);
    sctl = sctl&0;
    port->sctl = sctl;*/
}
void port_rebase(hba_port_t *port, int portno)
{

    stop_cmd(port);	// Stop command engine
    port->clb = AHCI_BASE + (portno<<10);
    port->fb = AHCI_BASE + (32<<10) + (portno<<8);
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)(port->clb);
    for (int i=0; i<32; i++)
    {
        cmdheader[i].prdtl = 8;
        cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);

    }

    port->cmd = (port->cmd)|0x10;//set FRE to 1
    port->cmd = (port->cmd)|0x10000002;//set SUD to 1
    while(1){
        if((port->ssts & 0x0F)>0){
                break;
        }
    }


//Dont touch this logic
    //Code for hardware
 /*   port->sctl |= 0x1;//Added
    kprintf("tfd 1 value %x\n",port->tfd);
    long count=0;
    while(1){
        count++;
        if(count==10000000000){
            break;
        }
    }
    //port->serr_rwc = 0xFFFFFFFF;
    port->serr_rwc = 0;
    port->is_rwc=0;
    port->sctl &= 0x0;//Added
    while(1){
        if((port->ssts&0x0F)>0){
            break;
        }
    }
    port->sctl |= 0x300;
    kprintf("tfd value %x\n",port->tfd);

*/





    start_cmd(port);	// Start command engine
}

// Check device type
static int check_type(hba_port_t *port)
{
    switch (port->sig)
    {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        case SATA_SIG_ATA:
            return AHCI_DEV_SATA;
        default:
            return AHCI_DEV_NULL;
    }
}

int probe_port(hba_mem_t *abar)
{
    // Search disk in impelemented ports
    int retvalue =0;
    unsigned int pi = abar->pi;
    int i = 0;
    while (i<32)
    {

        if (pi & 1)
        {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                kprintf("SATA drive found at port %d\n", i);
                abar->ghc |= 1;//HBA reset
                while(1){
                    if (abar->ghc & 1)
                        continue;
                    break;
                }
                abar->ghc |= (1<<31 | 2);
                sataDrivePort= &abar->ports[i];
                port_rebase(sataDrivePort,i);
                retvalue = 1;
                break;
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                kprintf("SATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB)
            {
                //kprintf("SEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM)
            {
                //kprintf("PM drive found at port %d\n", i);
            }
            else
            {
                //kprintf("No drive found at port %d\n", i);
            }
        }

        pi >>= 1;
        i ++;
    }
    return retvalue;

}

uint32_t pciConfigReadWord (unsigned short bus, unsigned short slot,
                            unsigned short func, unsigned short offset)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
                         (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl (CONFIG_ADDRESS, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (unsigned short)((inl (CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

uint32_t pciConfigRead (unsigned short bus, unsigned short slot,
                        unsigned short func, unsigned short offset)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
                         (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl (CONFIG_ADDRESS, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the 32 bits register */
    tmp = (unsigned int)(inl (CONFIG_DATA) );
    kprintf("tmp : %x\n",tmp);
    return (tmp);
}

int init_pci()
{
    int bus, slot, ahciFound = 0;
    unsigned short vendor, device, mout, mclass, msubclass,func;
    for (bus = 0; bus < 256; bus++){
        for (slot = 0; slot < 32; slot++){
            for(func = 0;func<7;func++)
            {
                vendor = pciConfigReadWord(bus, slot, func, (0x00 | 0x0));
                if (vendor == 65535)
                {
                    continue;
                }

                device = pciConfigReadWord(bus, slot, func, (0x00 | 0x02));
                if (device == 65535)
                {
                    continue;
                }

                mout = pciConfigReadWord(bus, slot, func, (0x00 | 0x0A));
                mclass = (unsigned char)(mout>>8);
                msubclass  = mout & 0xff;
                kprintf("device: %d, vendor: %x, sub: %x\n",device,vendor,msubclass);
                if(mclass == 0x01 && (msubclass == 0x05||msubclass == 0x06||msubclass == 0x07)){ // RM: AHCI
                    ahciFound = 1;
                    kprintf("AHCI device found \n");
                    break;
                }
            }
            if(ahciFound){
                break;
            }
        }

        if(ahciFound){
            break;
        }
    }
    uint64_t bar5 = pciConfigRead(bus, slot, func, (0x00 | 0x24));
    if(bar5 >= ADDRESS_SPACE_1GB) // temporary check
    {
        outl (CONFIG_DATA, ADDRESS_SPACE_REMAP);
        bar5 = (unsigned long)(inl (CONFIG_DATA) );
    }
    abar = (hba_mem_t*)((uint64_t *)bar5);
    if (probe_port(abar)){
        uint8_t *buf = (uint8_t *)BUFFER_SPACE;
        uint32_t startl = 20;
        //Code for hardware
        /*unsigned int ssts = sataDrivePort->ssts;
        unsigned char ipm = (ssts >> 8) & 0x0F;
        unsigned char det = ssts & 0x0F;
        kprintf("ipm %d, det %d\n",ipm,det);
        kprintf("hba cap value %p\n",abar->cap);
        kprintf("port cmd value %p\n",sataDrivePort->cmd);*/

        for(int count = 0 ; count < 100 ; count++,startl=startl+8){
            for( int i = 0 ; i< 4096 ; i++ )
                *(buf+i) = count;
           // write(sataDrivePort, startl, (uint32_t) 0, (uint32_t)1, buf);

        }
        for( int i = 0 ; i< 4096 ; i++ )
            *(buf+i) = 3;

        startl = 20;

        for(int count = 0 ; count < 100 ; count++,startl=startl+8 ){
           // read(sataDrivePort, startl, (uint32_t) 0, (uint32_t)1, buf);//113
            for(int i=2000;i<2020;i++){
              //  kprintf("%d ", *buf);
            }

            // kprintf("\n");
        }
    }

    return 0;
}



