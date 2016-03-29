/**
 * @FILENAME: analog.c
 * @BRIEF the source code of analog controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-30
 * @VERSION 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "adc.h"
#include "dac.h"
#include "arch_analog.h"
#include "drv_module.h"
#include "hal_sound.h"
#include "l2.h"
#include "akos_api.h"



#define LIN_GAIN_MAX    0xf
#define MIC_GAIN_MAX    0x7
#define HP_GAIN_MAX     0x6
#define ADC_MAIN_CLK    (12 * 1000000)

#define SIGNAL_PASS     0xff
#define POWER_ON        0x01
#define POWER_OFF       0x00

static T_BOOL m_adc1_init_flag = AK_FALSE;   //ADC1 init flag

static T_U32  hp_close_tick = 0;

static T_U32 sina_value[] = 
{
    #include "sina_value.h"
};


static void pnmos_open(void)
{
    //Open Nmos    
    REG32(ANALOG_CTRL_REG3) &= ~(PD_HP);

    //Fade out HP driver  here

    //open Pmos
//    REG32(ANALOG_CTRL_REG3) &= ~(0xf << PD_HP_CTRL);
    mini_delay(2);    
    REG32(ANALOG_CTRL_REG3) &= ~(PD2_HP);    
    REG32(ANALOG_CTRL_REG3) |= (0x08 << PD_HP_CTRL);
    mini_delay(1);
    
    REG32(ANALOG_CTRL_REG3) |= (0x04 << PD_HP_CTRL);
    mini_delay(1);
    
    REG32(ANALOG_CTRL_REG3) |= (0x02 << PD_HP_CTRL);
    mini_delay(1);
    
    REG32(ANALOG_CTRL_REG3) |= (0x01 << PD_HP_CTRL);
    mini_delay(1);
}

static T_VOID ms_delay(T_U32 minisecond)
{
    T_U32 tick;

    tick = get_tick_count() + minisecond;

#ifdef AKOS
    while(get_tick_count() < tick - 10)
    {
        AK_Sleep(1); //each tick 5ms
    }
#endif
    
    while(get_tick_count() < tick)
    {
        mini_delay(1);
    }
}


typedef struct 
{
    T_U16 charge_sel;
    T_U16 delay_ms;
}T_VCM2_CHARGE;

#define TOTAL_DELAY  360
static T_VOID VCM2_charging(T_VOID)
{    
    static T_VCM2_CHARGE value[] = 
    {
     //   {0x1E , 5 },   //5uA
     //   {0x10 , 2},   //7uA
        {0x0F , 2},   //12uA
        {0x0E , 2},   //15uA
        {0x07 , 2},   //17uA
        {0x0C , 2},   //20uA
        {0x06 , 2},   //24uA
        {0x03 , 2},   //29uA
        {0x08 , 2},   //36uA    
        {0x04 , 180},   //42uA        
    //    {0x02 , 1},   //50uA        
    //    {0x01 , 1},   //57uA
        
    //    {0x00 , 50},   //200uA
        
    //    {0x01 , 5},   //57uA        
    //    {0x02 , 5},   //50uA
    //    {0x04 , 5},   //42uA
        {0x08 , 5},   //36uA
        {0x03 , 5},   //29uA        
        {0x06 , 5 },   //24uA
        {0x0C , 5 },   //20uA        
        {0x07 , 5 },   //17uA
        {0x0E , 6},   //15uA
        {0x0F , 7 },   //12uA
   //     {0x10 , 8 },   //7uA
   //     {0x1E , 10 },   //5uA    

   //     {0x03 , 0}     //29uA , default value      
    };

    T_U32  reg_value;
    T_U32  uCount;
    T_U32  i;


    //set the bias the max for reduce the setup time
    REG32(ANALOG_CTRL_REG5) = 0x0;  
    
    //VCM2 charging current selection.    
    reg_value = REG32(ANALOG_CTRL_REG4);
    reg_value &= ~(0x1F << PTM_CHG);
    
    //5uA
    REG32(ANALOG_CTRL_REG4) = reg_value | (value[0].charge_sel << PTM_CHG);
    REG32(ANALOG_CTRL_REG3) &= ~(PL_VCM2);
    REG32(ANALOG_CTRL_REG3) &= ~(PD_VCM2);     
        
    ms_delay(value[0].delay_ms);

    uCount = value[0].delay_ms;


    for(i=1; i < sizeof(value)/sizeof(value[0]); ++i)
    {
        REG32(ANALOG_CTRL_REG4) = reg_value | (value[i].charge_sel << PTM_CHG);
        ms_delay(value[i].delay_ms);

        uCount = uCount + value[i].delay_ms;
    }

    if(uCount <= TOTAL_DELAY)
    {
        ms_delay(TOTAL_DELAY - uCount);
    }
    else
    {
        akprintf(C3 , M_DRVSYS, "Delay time too less\n");
    }

    //Set the charging current to default.
    REG32(ANALOG_CTRL_REG4) = reg_value | (0x03 << PTM_CHG);

    //set the the min for reduce the power    
    REG32(ANALOG_CTRL_REG5) = 0xFFFFFFFF;     
    
}


static T_VOID VCM2_discharging(T_VOID)
{
    
    static T_VCM2_CHARGE value[] = 
    {
        {0x01 , 20 },   //2.5uA
        {0x03 , 10 },   //7.5uA
        {0x07 , 10 },   //17.5uA        

        {0x0F , 310 },   //37.5uA   

        {0x1F , 80 },   //77.5uA   
        
        
        //{0x07 , 30 },   //17.5uA        
       // {0x03 , 30 },   //7.5uA
      //  {0x01 , 25 },   //2.5uA 

       // {0x07 , 1 },   //17.5uA    
      //  {0x0F , 1 },   //37.5uA   
    };

    T_U32  reg_value;
    T_U32  i;


    //set the bias the max for reduce the setup time
    REG32(ANALOG_CTRL_REG5) = 0x0; 
    
    //VCM2 discharging current selection.    
    reg_value = REG32(ANALOG_CTRL_REG3);
    reg_value &= ~(0x1F << PTM_DCHG);
    reg_value |= PD_VCM2;   //power down VCM2

    REG32(ANALOG_CTRL_REG3) = reg_value | (value[0].charge_sel << PTM_DCHG);        
    mini_delay(value[0].delay_ms);
    

    for(i=1; i < sizeof(value)/sizeof(value[0]); ++i)
    {
        REG32(ANALOG_CTRL_REG3) = reg_value | (value[i].charge_sel << PTM_DCHG);
        mini_delay(value[i].delay_ms);
    }

    //set the the min for reduce the power    
    REG32(ANALOG_CTRL_REG5) = 0xFFFFFFFF; 
    
    REG32(ANALOG_CTRL_REG3) |= PL_VCM2;  
    //mini_delay(11);
    
}


static T_VOID dac_output_sina(T_VOID)
{        
    T_U32       i;    
    SOUND_INFO  stInfo;
    T_U32       *addr_l2;        
    T_U32       *datebuf = sina_value;
    T_U32       data_len = sizeof(sina_value)/sizeof(sina_value[0]);
    T_U32       uCount;
    T_U8        bufid;

    
    bufid = l2_alloc(ADDR_DAC);    
    if(BUF_NULL == bufid)
    {
        akprintf(C2 , M_DRVSYS, "l2_alloc fail\n");
        return ;
    }

    //wait for all the music data having sended to DAC
    uCount = 0;
    while((REG32(L2_STAT_REG1) & (0xF << (bufid * 4))) != 0)
    {
#ifdef AKOS
        AK_Sleep(1);   
#else        
        mini_delay(5);
#endif        
        if(++uCount > 200)
        {
            akprintf(C3 , M_DRVSYS, "L2 not clear3\n");
            uCount = 0;
        }
    }
    

    stInfo.BitsPerSample = 16;
    stInfo.nChannel      = 2;
    stInfo.nSampleRate   = 8000;

    if(!dac_setinfo(&stInfo))
    {
        akprintf(C3 , M_DRVSYS, "dac_setinfo fail\n");
    }


    addr_l2 = (T_U32 *)(L2_BUF_MEM_BASE_ADDR + bufid*512); 


    
    //not mute
    REG32(DAC_CONFIG_REG) &= ~MUTE; 
    
    uCount = 0;     
    for(i = 0; i< data_len ; ++i)
    {        
        addr_l2[i & 127] = datebuf[i];    

        if((i & 0xF) == 0)
        {
            while(((REG32(L2_STAT_REG1) & (0xF << (bufid * 4))) >> (bufid * 4)) > 6)
            {
                mini_delay(1);
                if(++uCount > 1000)
                {
                    akprintf(C3 , M_DRVSYS, "L2 not clear1\n");
                    uCount = 0;
                }
            }
        }          
    }

    for(; (i & 0xF) != 0; ++i)
    {
        addr_l2[i & 127] = 0x80008000;  
    }

    uCount = 0;
    while((REG32(L2_STAT_REG1) & (0xF << (bufid * 4))) != 0)
    {
        mini_delay(1);
        if(++uCount > 1000)
        {
            akprintf(C3 , M_DRVSYS, "L2 not clear2\n");
            uCount = 0;
        }
    }     

    //mute
    REG32(DAC_CONFIG_REG) |= MUTE; 
    
}


static T_BOOL setconnect(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{   
    T_U32 reg_value;
    T_U32 hp_gain;

    
    if(SIGNAL_CONNECT == state)     //connect
    {
        if(analog_out & OUTPUT_ADC)
        {
            REG32(ANALOG_CTRL_REG4) |= analog_in << ADC2_IN;
        }

        if(analog_out & OUTPUT_HP)
        {            
            REG32(ANALOG_CTRL_REG3) |= analog_in << HP_IN;  
        }

        if(analog_out & OUTPUT_LINEOUT)
        {
            //do nothing
        }
        
    }
    else                            //disconnect
    {
        if(analog_out & OUTPUT_ADC)
        {
            REG32(ANALOG_CTRL_REG4) &= ~(analog_in << ADC2_IN);
        }

        if(analog_out & OUTPUT_HP)
        {
            REG32(ANALOG_CTRL_REG3) &= ~(analog_in << HP_IN);
        }

        if(analog_out & OUTPUT_LINEOUT)
        {
            //do nothing
        }
        
    }

    return AK_TRUE;
}


static T_VOID input_power_manage(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{
    T_U32 reg_value;
    T_U32 hp_input;
    T_U32 adc2_input;
    T_U32 lineout_input;
    T_U32 input;
    
    if(SIGNAL_CONNECT == state)     //connect
    {
        if(analog_in & INPUT_DAC)
        {
            //do nothing, power on dac in dac_open()
            akprintf(C3 , M_DRVSYS, "DAC ON in dac_open\n");
        }

        if(analog_in & INPUT_LINEIN)
        {            
            // power on the linein interface
            REG32(ANALOG_CTRL_REG4) &= ~(PD_LINEIN); 
            akprintf(C3 , M_DRVSYS, "LINEIN ON\n");
        }

        if(analog_in & INPUT_MIC)
        {                  
            // power on mic interface
            reg_value = REG32(ANALOG_CTRL_REG4);
            reg_value &= ~(PD_MICP | PD_MICN);  
            reg_value |= PD_MICN;  
            REG32(ANALOG_CTRL_REG4) = reg_value;
            akprintf(C3 , M_DRVSYS, "MIC ON\n");
        }
        
    }
    else                            //disconnect
    {
        hp_input        = (REG32(ANALOG_CTRL_REG3) >> HP_IN) & 0x07;
        adc2_input      = (REG32(ANALOG_CTRL_REG4) >> ADC2_IN) & 0x07;
        lineout_input   = 0;        //no lineout in AK37

        //analog_in is going to disconnect, so ignore it;
        if(analog_out & OUTPUT_ADC)
        {
            adc2_input &= ~analog_in;
        }

        if(analog_out & OUTPUT_HP)
        {         
            hp_input &= ~analog_in;
        }

        if(analog_out & OUTPUT_LINEOUT)
        {         
            lineout_input &= ~analog_in;
        }

        //analog_in does not connect output device, so to power off it.
        input = (hp_input | adc2_input | lineout_input);
        if((analog_in & INPUT_DAC) && (0 == (input & INPUT_DAC)))
        {
            //do nothing, power off dac in dac_close()
            akprintf(C3 , M_DRVSYS, "DAC OFF in dac_close\n");
        }

        if((analog_in & INPUT_LINEIN) &&(0 == (input & INPUT_LINEIN)))
        {
            // power off the linein interface
            REG32(ANALOG_CTRL_REG4) |= PD_LINEIN; 
            akprintf(C3 , M_DRVSYS, "LINEIN OFF\n");
        }

        if((analog_in & INPUT_MIC) && (0 == (input & INPUT_MIC)))
        {
            // power on mic interface
            REG32(ANALOG_CTRL_REG4) |= (PD_MICP | PD_MICN);
            akprintf(C3 , M_DRVSYS, "MIC OFF\n");
        }                
    }
}



#define MS_AFTER_CLOSE    200
static T_VOID output_power_manage(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{
    T_U32 reg_value;
    T_U32 hp_input;
    T_U32 adc2_input;
    T_U32 lineout_input;
    T_U32 input;
    T_U32 hp_gain;
    T_U32 tmp;

    hp_input        = (REG32(ANALOG_CTRL_REG3) >> HP_IN) & 0x07;
    adc2_input      = (REG32(ANALOG_CTRL_REG4) >> ADC2_IN) & 0x07;
    lineout_input   = 0;        //no lineout in AK37

    //analog_in is going to connect or disconnect, so ignore it;
    if(analog_out & OUTPUT_ADC)
    {
        adc2_input &= ~analog_in;
    }

    if(analog_out & OUTPUT_HP)
    {         
        hp_input &= ~analog_in;
    }

    if(analog_out & OUTPUT_LINEOUT)
    {         
        lineout_input &= ~analog_in;
    }

    if(SIGNAL_CONNECT == state) //connect
    {
        //power on ADC2
        if((analog_out & OUTPUT_ADC) && (0 == adc2_input))
        {
            REG32(ANALOG_CTRL_REG4) &= ~PD_ADC2;
            akprintf(C3 , M_DRVSYS, "ADC2 ON\n");
        }
        
        //power on hp
        if((analog_out & OUTPUT_HP) && (0 == hp_input))
        { 
            //if hp has power on ,do nothing
            reg_value = REG32(ANALOG_CTRL_REG3);
            if(0 == (reg_value & (PD_HP | PD2_HP)))
            {
                akprintf(C2 , M_DRVSYS, "HP has been powered on\n");
                return ;
            }

            tmp = get_tick_count();			
            if((tmp > hp_close_tick) && (tmp < hp_close_tick + MS_AFTER_CLOSE))
            {
                mini_delay(hp_close_tick + MS_AFTER_CLOSE - tmp);
            }            
            
            //VCM2 disable to pull down,
            //Not to discharge the off-chip AC coupling capacitor.
            REG32(ANALOG_CTRL_REG3) &= ~((0x1FU << PTM_DCHG) | (0x7U << Dischg_HP));
            
            //pull down resistor of hp ,All disable
            REG32(ANALOG_CTRL_REG3) &= ~(PRE_EN2 | PRE_EN1);
            
            //pull down resistor of VCM2 will be disable in VCM2_charging()
            
            //power on bias
            REG32(ANALOG_CTRL_REG3) &= ~(PD_BIAS); 

            //Disable the pull-down 2Kohm resistor to VCM3
            REG32(ANALOG_CTRL_REG4) &= ~(PL_VCM3);
            //Power on VCM3
            REG32(ANALOG_CTRL_REG3) &= ~( PD_VCM3);             

            //power on NMOS and PMOS
            pnmos_open(); 

            //power on VCM2 and charge.
            VCM2_charging();  

            if(INPUT_DAC & analog_in)
            {
                //power on the integrator in DAC and DAC CLK, after VCM2 chareging fully
                REG32(ANALOG_CTRL_REG3) &= ~(PD_CK | PD_OP);
                REG32(ANALOG_CTRL_REG3) |= RST_DAC_MID;  
                REG32(ANALOG_CTRL_REG3) &= ~RST_DAC_MID;              
            }
            
            akprintf(C3 , M_DRVSYS, "HP ON\n");
        }

        //power on lineout
        if((analog_out & OUTPUT_LINEOUT) && (0 == lineout_input))
        {
            //do nothing
        }
    }
    else                        //disconnect
    {
        //power off ADC2
        if((analog_out & OUTPUT_ADC) && (0 == adc2_input))
        {
            REG32(ANALOG_CTRL_REG4) |= PD_ADC2;
            akprintf(C3 , M_DRVSYS, "ADC2 OFF\n");
        }
        
        //power off hp
        if((analog_out & OUTPUT_HP) && (0 == hp_input))
        {
            //if hp has power off ,do nothing
            reg_value = REG32(ANALOG_CTRL_REG3);
            if((PD_HP | PD2_HP) == (reg_value & (PD_HP | PD2_HP)))
            {
                akprintf(C2 , M_DRVSYS, "HP is not power on\n");
                return ;
            }

            //HP input include DAC
            if((INPUT_DAC & analog_in) && 
                (0 == (reg_value & (PD_CK | PD_OP))))
            {                
                //set hp gain to 0db
                reg_value = REG32(ANALOG_CTRL_REG3);
                hp_gain   = reg_value & (0x1F << HP_GAIN);  //save hp gain
                reg_value &= ~(0x1F << HP_GAIN);
                reg_value |= 0x10 << HP_GAIN;
                REG32(ANALOG_CTRL_REG3) = reg_value;
                
                //dac to hp connect , just in case
                reg_value = REG32(ANALOG_CTRL_REG3);
                reg_value &= ~(7 << HP_IN);
                reg_value |= (1<<HP_IN);
                REG32(ANALOG_CTRL_REG3) = reg_value;

                dac_output_sina(); 

                //Discharging HP by discharging VCM2
                reg_value = REG32(ANALOG_CTRL_REG3);
                reg_value |= (PD_VCM2);
                //REG32(ANALOG_CTRL_REG3) = reg_value | (0x03 << PTM_DCHG);            
                //ms_delay(60);
                REG32(ANALOG_CTRL_REG3) = reg_value | (0x01 << PTM_DCHG);
                ms_delay(160);

                //power off PMOS & NMOS
                //At the same time, DAC to HP disconnection          
                reg_value = REG32(ANALOG_CTRL_REG3);
                reg_value |= PD2_HP | PD_HP; 
                reg_value &= ~(1<<HP_IN); 
                REG32(ANALOG_CTRL_REG3) = reg_value;

                //power off  the integrator in DAC and DAC CLK, after DAC is not used
                REG32(ANALOG_CTRL_REG3) |= PD_CK | PD_OP;                
                REG32(ANALOG_CTRL_REG3) |= RST_DAC_MID;  

                //VCM2 discharging fast.    
                reg_value = REG32(ANALOG_CTRL_REG3);
                reg_value |= (0x1F << PTM_DCHG);
                reg_value |= PD_VCM2;   //power down VCM2
                reg_value |= PL_VCM2;
                REG32(ANALOG_CTRL_REG3) |= reg_value;                 

                //resume the hp gain
                reg_value = REG32(ANALOG_CTRL_REG3);
                reg_value &= ~(0x1F << HP_GAIN);
                reg_value |= hp_gain;
                REG32(ANALOG_CTRL_REG3) = reg_value;                  
                
                hp_close_tick = get_tick_count();
                
            }
            else                            //HP input is not include DAC 
            {
                //hp mute                
                REG32(ANALOG_CTRL_REG3) &= ~(7 << HP_IN);
                
                if(INPUT_DAC & analog_in)
                {
                    akprintf(C3 , M_DRVSYS, 
                        "HP should be power off before DAC close\n");
                    
                    //power off  the integrator in DAC and DAC CLK,  just in case
                    REG32(ANALOG_CTRL_REG3) |= PD_CK | PD_OP;                     
                    REG32(ANALOG_CTRL_REG3) |= RST_DAC_MID;  
                }                 

                //power off VCM2 and discharge it.After this, HP no need to discharge
                VCM2_discharging();

                //power off PMOS & NMOS
                REG32(ANALOG_CTRL_REG3) |= PD2_HP | PD_HP;  

                hp_close_tick = 0;                
            }

            //Set HP at the discharging state
            REG32(ANALOG_CTRL_REG3) |= PRE_EN1 | PRE_EN2;

            //Power off VCM3 
            REG32(ANALOG_CTRL_REG3) |= PD_VCM3;
            //Enable the pull-down 2Kohm resistor to VCM3
            REG32(ANALOG_CTRL_REG4) |= PL_VCM3;

            //power off  bias
            REG32(ANALOG_CTRL_REG3) |= PD_BIAS; 

            akprintf(C3 , M_DRVSYS, "HP OFF\n");
        }
        
        //power off lineout
        if((analog_out & OUTPUT_LINEOUT) && (0 == lineout_input))
        {
            //do nothing
            akprintf(C3 , M_DRVSYS, "LINEOUT OFF\n");
        }
    }
    
}


/**
 * @brief   connect or disconnect the signal between input and output signal. 
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT
 * @param[in] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL  analog_setsignal(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE state)
{
    if((analog_in > INPUT_ALL) || (analog_in < INPUT_DAC) || 
       (analog_out > OUTPUT_ALL) || (analog_out < OUTPUT_ADC))
    {
        akprintf(C2, M_DRVSYS, "analog_in or analog_out is error\n");
        return AK_FALSE;
    }

    DrvModule_Protect(DRV_MODULE_DA); 

    if(SIGNAL_CONNECT == state)     //connect
    {
        input_power_manage(analog_in, analog_out, state);   
        //because linein interface occur a mistake state while vcm2 charging,
        //so do specially.
        if((INPUT_LINEIN & analog_in))
        {
            output_power_manage(analog_in, analog_out, state);            
            setconnect(analog_in, analog_out, state);        
        }
        else
        {            
            output_power_manage(analog_in, analog_out, state);  
	        setconnect(analog_in, analog_out, state);

        }
    }
    else
    {
        output_power_manage(analog_in, analog_out, state);
        setconnect(analog_in, analog_out, state);
        input_power_manage(analog_in, analog_out, state);
    }

    DrvModule_UnProtect(DRV_MODULE_DA); 
    return AK_TRUE;    
}


/**
 * @brief   connect or disconnect the signal between input and output signal. 
 * @author  WangGuotian
 * @date    2012-05-14
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT
 * @param[in] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_setconnect(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{
    if((analog_in > INPUT_ALL) || (analog_in < INPUT_DAC) || 
       (analog_out > OUTPUT_ALL) || (analog_out < OUTPUT_ADC))
    {
        akprintf(C2, M_DRVSYS, "analog_in or analog_out is error\n");
        return AK_FALSE;
    }

    DrvModule_Protect(DRV_MODULE_DA); 

    setconnect(analog_in, analog_out, state);

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return AK_TRUE;
}




/**
 * @brief   get the signal connection state between input and output source
 * @author  WangGuotian
 * @date    2012-05-14
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT 
 * @param[out] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_getsignal(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE *state)
{
    T_U32 input;
    
    if(analog_in > 0x07)//0~0x07
    {
        akprintf(C2, M_DRVSYS, "signal input value > 7!\n");
        return AK_FALSE;
    }

    if(analog_out > 0x07)//0~0x07
    {
        akprintf(C2, M_DRVSYS, "signal output value > 7!\n");
        return AK_FALSE;
    }

    DrvModule_Protect(DRV_MODULE_DA); 

    *state = SIGNAL_DISCONNECT;

    if(OUTPUT_ADC == analog_out)
    {
        input = (REG32(ANALOG_CTRL_REG3) >> HP_IN) & 0x07;
    }
    else if(OUTPUT_HP == analog_out)
    {
        input = (REG32(ANALOG_CTRL_REG4) >> ADC2_IN) & 0x07;
    }
    else if(OUTPUT_LINEOUT == analog_out)
    {
        input = 0;
    }
    else
    {
        input = 0;
    }    

    if(input & analog_in)
    {
        *state = SIGNAL_CONNECT;
    } 

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return AK_TRUE;
}


/**
 * @brief   set analog module channel to be MONO or STEREO
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] module refer to ANALOG_CHANNEL
 * @param[in] state CHANNEL_MONO or CHANNEL_STEREO
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL    analog_setchannel(ANALOG_CHANNEL module, ANALOG_CHANNEL_STATE    state)
{
    akprintf(C3, M_DRVSYS, "not supply now\n");
    return AK_FALSE;
}

/**
 * @brief   get signal channel state, MONO or STEREO
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in]  module refer to ANALOG_CHANNEL
 * @param[out] state CHANNEL_MONO or CHANNEL_STEREO
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_getchannel(ANALOG_CHANNEL module, ANALOG_CHANNEL_STATE *state)
{
    akprintf(C3, M_DRVSYS, "not supply now\n");
    return AK_FALSE;
}

/**
 * @brief   Set headphone gain,available for aspen3s later
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] gain for normal mode, must be 0~8.0 for mute,1~8 for 0.1 time to 0.8 time
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_setgain_hp (T_U8 gain)
{
    T_U32 reg_value;
    T_U32 gain_table[6] = {0x0, 0x18, 0x14, 0x12, 0x11, 0x10};

    if(gain >= HP_GAIN_MAX)
    {
        akprintf(C3, M_DRVSYS, "set gain bigger than %d\n", HP_GAIN_MAX);
        return AK_FALSE;
    }
    
    DrvModule_Protect(DRV_MODULE_DA);

    reg_value = REG32(ANALOG_CTRL_REG3);
    reg_value &= ~(0x1F << HP_GAIN);
    reg_value |= (gain_table[gain] << HP_GAIN);
    REG32(ANALOG_CTRL_REG3) = reg_value;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return AK_TRUE;
}

/**
 * @brief   Set mic gain
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] gain must be 0~3,(aspen3s:0~7).
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_setgain_mic(T_U8 gain)
{
    T_U32 reg_value;
    
    if(gain > MIC_GAIN_MAX)
    {
        akprintf(C3, M_DRVSYS, "set gain bigger than %d\n", MIC_GAIN_MAX);
        return AK_FALSE;
    }
    
    DrvModule_Protect(DRV_MODULE_DA); 

    reg_value = REG32(ANALOG_CTRL_REG4);
    reg_value &= ~(MIC_GAIN_MAX << 9);//10
    reg_value |= (gain << 9);
    REG32(ANALOG_CTRL_REG4) = reg_value;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return AK_TRUE;
}

/**
 * @brief   Set linein gain
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] gain must be 0~3,1 is 0db(aspen3s:0~15,6 is 0db)
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_setgain_linein(T_U8 gain)
{
    T_U32 reg_value;
    if(gain > LIN_GAIN_MAX)
    {
        akprintf(C3, M_DRVSYS, "set gain bigger than %d\n", LIN_GAIN_MAX);
        return AK_FALSE;
    }

    DrvModule_Protect(DRV_MODULE_DA); 

    reg_value = REG32(ANALOG_CTRL_REG4);
    reg_value &= ~(LIN_GAIN_MAX << 5);
    reg_value |= (gain << 5);
    REG32(ANALOG_CTRL_REG4) = reg_value;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return AK_TRUE;
}


T_VOID analog_adc1_init(T_U32 ADCClock, T_U32 SampleRate)
{
    T_U32 ClkDiv = 0;
    T_U32 bitcycle = 0;
    T_U32 WaitTime;
    T_U32 HoldTime;
    static T_U32 s_ADCClock, s_SampleRate;

    //if first init, save param setting
    if (!m_adc1_init_flag)
    {
        s_ADCClock = ADCClock;
        s_SampleRate = SampleRate;
    }

    //if setting changes, reinitial ADC1
    if (s_ADCClock != ADCClock || s_SampleRate != SampleRate)
    {
        m_adc1_init_flag = AK_FALSE;
        s_ADCClock = ADCClock;
        s_SampleRate = SampleRate;
    }        
    
    if(!m_adc1_init_flag)
    {
        akprintf(C3, M_DRVSYS, "init adc1 clock:%d, samplerate:%d\n", ADCClock, SampleRate);
        		
        //reset ADC1
        REG32(ANALOG_CTRL_REG1) |= (1 << 1); 
        mini_delay(10);//delay for reset
        REG32(ANALOG_CTRL_REG1) &= ~(1 << 1); 

        //set adc clk and enable adc1 clk
        ClkDiv = (ADC_MAIN_CLK / ADCClock) - 1;
        ClkDiv &= 0x7;
        REG32(ADC_CLK_DIV) &= ~(0x7U << 29);
        REG32(ADC_CLK_DIV) |= (1 << 3) | (ClkDiv << 0);	//enable adc1clk

        //clear powerdown
		REG32(ANALOG_CTRL_REG1) &= ~(1 << 0);
            
        /* because ADC1 is 5 channel multiplex*/
        SampleRate = SampleRate * 5;

        bitcycle = (T_U32)(ADCClock / SampleRate);
        HoldTime =  bitcycle - 1;  
        REG32(ADC_CONTROL1) =  (HoldTime << 16) | bitcycle;    //bitcycle; HoldTime±ØÐëÅäÖÃ
        mini_delay(10);//delay 
   
        //Enable ADC1
        REG32(ANALOG_CTRL_REG2) |= (1 << 8);

    }
     //init over, keep it.
    m_adc1_init_flag = AK_TRUE;
}

/**
 * @brief get adc1 ad4 value. if input voltage from 0 to AVDD, it will return the value from 0 to 1023 
 * @author  Liangenhui 
 * @date 2010-07-30
 * @return T_U32
 */
T_U32 analog_getvalue_bat(T_VOID)
{
    T_U32 ad4_value = 0;
    T_U32 count, reg_data;

    DrvModule_Protect(DRV_MODULE_AD); 

    //it set define if touch scheen not initial
    analog_adc1_init(4000000,  1000);
    
    //base on  spec
    mini_delay(10);

    reg_data = REG32(ANALOG_CTRL_REG2);
    //enable battery monitor
    reg_data |= (1 << 9);
    reg_data &= ~(1<<11);   //disable AIN channel
    REG32(ANALOG_CTRL_REG2) = reg_data;
    
    reg_data = REG32(ANALOG_CTRL_REG1);
    //select VBAT
	reg_data &= ~(0x1F<<3);
    reg_data |= (0x07<<4);  //d (2), 7 (3)
    
    REG32(ANALOG_CTRL_REG1) = reg_data;

    count = 5;
    //sample 5 times
    while ( count-- )
    {
        us_delay(1000);//delay for get next point ad4
        ad4_value += ((REG32(ADC1_STAT_REG) >> 10) & 0x3ff);//0x3ff for 10 bit adc
    }

    //deselect VBAT mode
    REG32(ANALOG_CTRL_REG1) &= ~(0x1F << 3); //»Ö¸´ÉèÖÃ£¬ÒÔÃâÓ°Ïì°´¼ü
    
    //disable battery monitor
    REG32(ANALOG_CTRL_REG2) &= ~(1 << 9);

    DrvModule_UnProtect(DRV_MODULE_AD); 

    return ad4_value / 5;
}

/**
 * @brief get adc1 ad5 value. if input voltage from 0 to AVDD, it will return the value from 0 to 1023 
 * @author  Liangenhui 
 * @date 2010-07-30
 * @return T_U32
 */
T_U32 analog_getvalue_ad5(T_VOID)
{
    T_U32 ad5_value = 0;
    T_U32 reg_data;
	
    DrvModule_Protect(DRV_MODULE_AD); 
    
    analog_adc1_init(4*1000*1000, 1000);

    irq_mask();
    //select one channel mode
    REG32(ANALOG_CTRL_REG2) |= (1 << 11); 
    
    //select AIN 
    reg_data = REG32(ANALOG_CTRL_REG1);
    reg_data &= ~(1<<4);    //disable battery monitor
    reg_data |= (1<<3);     //select keypad singal from AIN channel to ADC1 for data conversion
    REG32(ANALOG_CTRL_REG1) = reg_data;
   
    irq_unmask();
    
    us_delay(1000);//delay for get ad5 value
  
    irq_mask();

	ad5_value = REG32(ADC1_STAT_REG) & 0x3ff;

    //deselect AIN mode
    REG32(ANALOG_CTRL_REG1) &= ~(3 << 3);

    irq_unmask();

    DrvModule_UnProtect(DRV_MODULE_AD); 

    return ad5_value;
}

/**
 * @brief set the mode of DAC analog voltage, it can be set to AC mode or DC mode
 * @author  Liangenhui 
 * @date 2010-07-30
 * @param[in] mode:must be MODE_AC or MODE_DC
 * @return T_BOOL  
 */
T_BOOL analog_setmode_voltage(ANALOG_VOLTAGE_MODE mode)
{
#if 0
    if(MODE_DC == mode)
    {
        REG32(TS_CONTROL_REG1) &= ~(1 << 22);
    }
    else if(MODE_AC == mode)
    {
        REG32(TS_CONTROL_REG1) |= (1 << 22);
    }
    else
    {
        return AK_FALSE;
    }
#endif
    return AK_TRUE;
}   


