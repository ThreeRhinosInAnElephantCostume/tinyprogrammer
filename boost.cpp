    // This file is part of tinyavrprogrammer.

    // tinyavrprogrammer is free software: you can redistribute it and/or modify
    // it under the terms of the GNU General Public License as published by
    // the Free Software Foundation, version 3.

    // tinyavrprogrammer is distributed in the hope that it will be useful,
    // but WITHOUT ANY WARRANTY; without even the implied warranty of
    // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    // GNU General Public License for more details.

    // You should have received a copy of the GNU General Public License
    // along with tinyavrprogrammer.  If not, see <https://www.gnu.org/licenses/>.

#include "hmain.hpp"
// declared globally for the purpose of debugging
float VBUSV = 0.0f;
float V = 0.0f;
int duty = BOOST::DEFAULT_DUTY;

float get_vbus()
{
    return adc_read_voltage(ADC::VBUSREAD, ADC::VBUSREAD_MP);
}
float get_vboost()
{
    return adc_read_voltage(ADC::BOOSTREAD, ADC::BOOSTREAD_MP) + ADC::BOOSTREAD_OFFSET;
}
void disable_power()
{
    gpio_put(PIN::HIGHVOLT, 0);
    gpio_put(PIN::POWER, 0);
    pwm_set_enabled(pwm_gpio_to_slice_num(PIN::PULSE), false);
    sleep_us(1);
    gpio_put(PIN::PULSE, 0);
}
void PANIC()
{
    panicnow = true;
    disable_power();
    gpio_put(PIN::HIGHVOLT, 0);
    gpio_put(PIN::POWER, 0);
    if(DEBUG)
    {
        printf("PANIC!!\n");
        printf("\nV: %.4f\n", V);
        printf("VBUS: %.4f\n", VBUSV);
        printf("duty: %i\n", duty);
        printf("\nPANIC!!\n");
}
    if(!DEBUG)
        watchdog_enable(0, false);
    while(true)
    {
        gpio_put(PIN::HIGHVOLT, 0);
        gpio_put(PIN::POWER, 0);
        if(!DEBUG)
            printf("PANIC!!\n");
    }
}
void safe_power()
{
    ispowersafe = true;
}
void unsafe_power(bool emergency=false)
{
    ispowersafe = false;
    if(emergency && chippowered)
    {
        PANIC();
    }
    // gpio_put(PIN::POWER, 0);
    // gpio_put(PIN::HIGHVOLT, 0);
}
void tick_power()
{
    bool vbussafe = false;
    VBUSV = get_vbus();
    if(VBUSV > BOOST::VBUS_MAXIMUM)
    {
        unsafe_power(true);    
    }
    else if(VBUSV < BOOST::VBUS_MINIMUM)
    {
        unsafe_power();
    }
    else 
        vbussafe = true;
    bool vboostsafe = false;
    V = get_vboost();
    if(V > BOOST::OVERVOLT || V < BOOST::UNDERVOLT)
    {
        unsafe_power(true);
    }
    else if(V < BOOST::MIN_SAFE || V > BOOST::MAX_SAFE)
    {
        unsafe_power();
    }
    else
        vboostsafe = true;
    if(vboostsafe && vbussafe && gettime() > 1000*1000)
        safe_power();
    boostvoltage = V;
    uint16_t change = std::min(BOOST::MAXCHANGE,
         (uint16_t)std::max((int)BOOST::MINCHANGE, abs((uint16)((BOOST::TARGET-V) * (float)BOOST::CHANGEBASE)) ));
    if(V > BOOST::TARGET)
    {
        duty -= change;
    }
    else if (V < BOOST::TARGET)
    {
        duty += change;
    }
    if(duty > BOOST::MAX_DUTY || duty > BOOST::WRAP)
        duty = BOOST::MAX_DUTY;
    else if(duty < BOOST::MIN_DUTY || duty < 0)
        duty = BOOST::MIN_DUTY;
    pwm_set_gpio_level(PIN::PULSE, (uint16)duty);

    
    if(DEBUG && printpower)
    {
        EVERY_N(1000)
        {
            clear_console();
            printf("V: %.3f\n", V);
            printf("VBUS: %.3f\n", VBUSV);
            printf("duty: %i\n", duty);
            printf("change: %i\n", change);
            printf("ispowersafe: %s\n", (ispowersafe)? "true": "false");
        }
    }

}
void init_power_control()
{
    gpio_init(PIN::PULSE);
    gpio_set_function(PIN::PULSE, GPIO_FUNC_PWM);

    init_adc_pin(ADC::BOOSTREAD);
    init_adc_pin(ADC::VBUSREAD);

    uint slice_num = pwm_gpio_to_slice_num(PIN::PULSE);
    pwm_set_wrap(slice_num, BOOST::WRAP);
    pwm_set_gpio_level(PIN::PULSE, BOOST::DEFAULT_DUTY);
    pwm_set_clkdiv_int_frac(slice_num, BOOST::DIVIDER, 0);
    pwm_set_enabled(slice_num, true);
}