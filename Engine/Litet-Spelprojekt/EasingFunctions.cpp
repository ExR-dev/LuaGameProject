#include "stdafx.h"
#include "EasingFunctions.h"

#include <cmath>
#include <iostream>

const float c1 = 1.70158f;
const float c2 = c1 * 1.525f;
const float c3 = c1 + 1;
const float c4 = (2 * M_PI) / 3;
const float c5 = 2 * M_PI / 4.5f; 

const float n1 = 7.5625f;
const float d1 = 2.75f;

float EaseInSine(float fraction)
{
    return 1 - cos((fraction * M_PI) / 2);
}

float EaseOutSine(float fraction)
{
    return sin((fraction * M_PI) / 2);
}

float EaseInOutSine(float fraction)
{
    return -(cos(fraction * M_PI) - 1) / 2;
}

float EaseInQuad(float fraction)
{
    return fraction * fraction;
}

float EaseOutQuad(float fraction)
{
    return 1 - (1 - fraction) * (1 - fraction);
}

float EaseInOutQuad(float fraction)
{
    if (fraction < 0.5f)
        return 2 * fraction * fraction;
    return 1 - pow(-2 * fraction + 2, 2) / 2;
}

float EaseInCubic(float fraction)
{
    return pow(fraction, 3);
}

float EaseOutCubic(float fraction)
{
    return 1 - pow(1 - fraction, 3);
}

float EaseInOutCubic(float fraction)
{
    if (fraction < 0.5f)
        return 4 * pow(fraction, 3);
    return 1 - pow(-2 * fraction + 2, 3) / 2;
}

float EaseInQuart(float fraction)
{
    return fraction * fraction * fraction * fraction;
}

float EaseOutQuart(float fraction)
{
    return 1 - pow(1 - fraction, 4);
}

float EaseInOutQuart(float fraction)
{
    if (fraction < 0.5f)
        return 8 * fraction * fraction * fraction * fraction;
    return 1 - pow(-2 * fraction + 2, 4) / 2;
}

float EaseInQuint(float fraction)
{
    return fraction * fraction * fraction * fraction * fraction;
}

float EaseOutQuint(float fraction)
{
    return 1 - pow(1 - fraction, 5);
}

float EaseInOutQuint(float fraction)
{
    if (fraction < 0.5f)
        return 16 * fraction * fraction * fraction * fraction * fraction;
    return 1 - pow (-2 * fraction + 2, 5) / 2;
}

float EaseInExpo(float fraction)
{
    if (fraction <= 0)
        return 0;
    return pow(2, 10 * fraction - 10);
}

float EaseOutExpo(float fraction)
{
    if (fraction >= 1)
        return 1;
    return 1 - pow(2, -10 * fraction);
}

float EaseInOutExpo(float fraction)
{
    if (fraction <= 0)
        return 0;
    if (fraction >= 1)
        return 1;
    if (fraction < 0.5f)
        return pow(2, 20 * fraction - 10) / 2;
    return (2 - pow(2, -20 * fraction + 10)) / 2;
}

float EaseInCirc(float fraction)
{
    if (fraction >= 1)
        return 1;
    if (fraction <= 0)
        return 0;
    return 1 - sqrt(1 - pow(fraction, 2));
}

float EaseOutCirc(float fraction)
{
    if (fraction >= 1)
        return 1;
    if (fraction <= 0)
        return 0;
    return sqrt(1 - pow(fraction - 1, 2));
}

float EaseInOutCirc(float fraction)
{
    if (fraction >= 1)
        return 1;
    if (fraction <= 0)
        return 0;
    if (fraction < 0.5f)
        return (1 - sqrt(1 - pow(2 * fraction, 2))) / 2;
    return (sqrt(1 - pow(-2 * fraction + 2, 2)) + 1) / 2;
}

float EaseInBack(float fraction)
{
    return c3 * fraction * fraction * fraction - c1 * fraction * fraction;
}

float EaseOutBack(float fraction)
{
    return 1 + c3 * pow(fraction - 1, 3) + c1 * pow(fraction - 1, 2);
}

float EaseInOutBack(float fraction)
{
    if (fraction <= 0)
        return 0;
    if (fraction >= 1)
        return 1;
    if (fraction < 0.5f)
        return (pow(2 * fraction, 2) * ((c2 + 1) * 2 * fraction - c2)) / 2;
    return (pow(2 * fraction - 2, 2) * ((c2 + 1) * (fraction * 2 - 2) + c2) +2) / 2;
}

float EaseInElastic(float fraction)
{
    if (fraction <= 0)
        return 0;
    else if (fraction >= 1)
        return 1;
    return -pow(2, 10 * fraction - 10) * sin((fraction * 10 - 10.75) * c4);
}

float EaseOutElastic(float fraction)
{
    if (fraction <= 0)
        return 0;
    else if (fraction >= 1)
        return 1;
    return -pow(2, 10 * fraction - 10) * sin((fraction * 10 - 10.75f) * c4);
}

float EaseInOutElastic(float fraction)
{
    if (fraction <= 0)
        return 0;
    else if (fraction >= 1)
        return 1;
    else if (fraction < 0.5f)
        return (-(pow(2, 20 * fraction - 10) * sin((20 * fraction - 11.125f) * c5)) / 2);
    return ((pow(2, -20 * fraction + 10) * sin((20 * fraction - 11.125f) * c5)) / 2 + 1);
}

float EaseInBounce(float fraction)
{
    return 1 - EaseOutBounce(1 - fraction);
}

float EaseOutBounce(float fraction)
{
    float tempFrac = fraction;

    if (tempFrac < 1 / d1) 
        return n1 * tempFrac * tempFrac;
    else if (tempFrac < 2 / d1) 
    {
        tempFrac -= 1.5f / d1;
        return n1 * (tempFrac) * tempFrac + 0.75f;
    }
    else if (tempFrac < 2.5f / d1) 
    {
        tempFrac -= 2.25f / d1;
        return n1 * (tempFrac) * tempFrac + 0.9375f;
    }
    tempFrac -= 2.625f / d1;
    return n1 * (tempFrac / d1) * tempFrac + 0.984375f;
}

float EaseInOutBounce(float fraction)
{
    if (fraction < 0.5f)
        return (1 - EaseOutBounce(1 - 2 * fraction)) / 2;
    return (1 + EaseOutBounce(2 * fraction - 1)) / 2;
}