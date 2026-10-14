#include "color_engine.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace techni;
int checks=0;
void check(bool pass,const char* label) { ++checks; if(!pass) { std::cerr<<"FAIL: "<<label<<'\n'; std::exit(1); } }
bool near(double a,double b,double eps=1e-6) { return std::abs(a-b)<eps; }
bool same(Color a,Color b) { return near(a.r,b.r)&&near(a.g,b.g)&&near(a.b,b.b)&&near(a.a,b.a); }
int main() {
    Random rng(400);
    const double reference[]{.6963689456211258,.18477239561489428,.36321888042763756,.9353334256146725,.800329060200755,.11800746764894923};
    for(double value:reference) check(near(rng.sample(),value,1e-15),"PC .NET Random(400) sequence");
    check(same(hsv(0),{1,0,0}),"red at hue zero");
    check(same(hsv(1.f/3),{0,1,0}),"green sector");
    check(same(hsv(2.f/3),{0,0,1}),"blue sector");
    check(same(hsv(-.25),hsv(.75)),"negative hue wraps");
    check(same(hsv(1),hsv(0)),"full turn wraps");
    check(same(palette(warmPalette,3.5),lerp(warmPalette[3],warmPalette[0],.5)),"last palette entry wraps to first");
    check(same(palette(coldPalette,-.5),lerp(coldPalette[3],coldPalette[0],.5)),"negative palette time");
    Random flatRng(400);
    auto flatColor=flat(true,Style::WarmCold,flatRng);
    check(same(flatColor,{1,0,.69636894f}),"preserve original PC first-pair interpolation");
    Options o;
    check(o.enabled&&o.lights==Style::Gradient&&o.walls==Style::Gradient&&o.bombs==Style::PureRandom,"PC defaults");
    check(same(gradient(o,0,true),hsv(.5)),"PC Desync false applies half-cycle left offset");
    o.desync=true;
    check(same(gradient(o,3,true),gradient(o,3,false)),"PC Desync true removes offset");
    o.useLeftNotes=true; o.leftNotes=Style::WarmCold; o.notes=Style::Gradient;
    o.useLeftSaber=true; o.leftSaber=Style::Off; o.sabers=Style::PureRandom;
    check(o.noteStyle(true)==Style::WarmCold&&o.noteStyle(false)==Style::Gradient,"independent left notes");
    check(o.saberStyle(true)==Style::Off&&o.saberStyle(false)==Style::PureRandom,"independent left saber off");
    check(same(lightColor({0,0,0,0},hsv(.3),1),{0,0,0,0}),"off lighting remains off");
    check(same(lightColor({.2,.1,.05,.4},{0,1,0},.5),{0,.2,0,.6}),"preserve fading intensity and boost alpha");
    check(same(lightColor({2,.1,0,.5},{0,0,1},0),{0,0,2,.5}),"preserve HDR intensity");
    check(validStyle(-1)==Style::Off&&validStyle(99)==Style::Off,"invalid enum is off");
    check(near(finiteClamp(std::numeric_limits<float>::quiet_NaN(),0,1,.1f),.1),"NaN config fallback");
    check(near(finiteClamp(9,0,1,.1f),1),"config range clamp");
    std::cout<<checks<<" color contract checks passed\n";
}
