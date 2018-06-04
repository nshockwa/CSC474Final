#pragma once
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

class keyframe
{
public:
    quat quaternion;
    vec3 translation;
    long long timestamp_ms;
};
class animation_per_bone
{
public:
    string name;
    long long duration;
    int frames;
    string bone;
    vector<keyframe> keyframes;
};
class all_animations
{
public:
    vector<animation_per_bone> animations;

};


//**************************************************

class bone
{
public:
    vector<animation_per_bone*> animation;    //all the animations of the bone
    string name;
    vec3 pos;
    quat q;
    bone *parent = NULL;
    vector<bone*> kids;
    unsigned int index;            //a unique number for each bone, at the same time index of the animatiom matrix array
    mat4 *mat = NULL;            //address of one lement from the animation matrix array
    // searches for the animation and sets the animation matrix element to the recent matrix gained from the keyframe
    void play_animation(float keyframenumber, string animationname, float inter)
    {
      float t = keyframenumber - int(keyframenumber);
                if (animation[1]->keyframes.size() > keyframenumber)
                {
                    float ratio = 1. * animation[1]->keyframes.size() / animation[3]->keyframes.size();
                    int framea = (int)keyframenumber;
                    int frameb = (int)keyframenumber + 1;
                    //quat q = animation[i]->keyframes[keyframenumber].quaternion;
                    quat qa = animation[1]->keyframes[framea].quaternion;
                    quat qb = animation[1]->keyframes[frameb].quaternion;
                    quat qr1 = slerp(qa,qb,t);
                    quat qc = animation[3]->keyframes[framea/ratio].quaternion;
                    quat qd = animation[3]->keyframes[frameb/ratio].quaternion;
                    quat qr2 = slerp(qc,qd,t);
                    quat qrf = slerp(qr1,qr2,inter);
                    //vec3 tr = animation[i]->keyframes[keyframenumber].translation;
                    vec3 ta = animation[1]->keyframes[framea].translation;
                    vec3 tb = animation[1]->keyframes[frameb].translation;
                    vec3 tr1 = mix(ta,tb,t);
                    vec3 tc = animation[3]->keyframes[framea/ratio].translation;
                    vec3 td = animation[3]->keyframes[frameb/ratio].translation;
                    vec3 tr2 = mix(tc,td,t);
                    vec3 trf = mix(tr1,tr2,inter);

                    /*if (name == "dragon2" || name == "masterController"
                      || name == "Armature" || name == "Bone_002" )
                        trf = vec3(0, 0, 0);*/
                    mat4 M = mat4(qrf);
                    mat4 T = translate(mat4(1), trf);
                    M = T * M;
                    if (mat)
                    {
                        mat4 parentmat = mat4(1);
                        if (parent)
                            parentmat = *parent->mat;
                        *mat = parentmat * M;
                    }
                else
                    *mat = mat4(1);
        for (int i = 0; i < kids.size(); i++)
            kids[i]->play_animation(keyframenumber,animationname, inter);
          }
    }



    //writes into the segment positions and into the animation index VBO
    void write_to_VBOs(vec3 origin, vector<vec3> &vpos, vector<unsigned int> &imat)
    {
        vpos.push_back(origin);
        vec3 endp = origin + pos;
        vpos.push_back(endp);

        if(parent)
            imat.push_back(parent->index);
        else
            imat.push_back(index);
        imat.push_back(index);

        for (int i = 0; i < kids.size(); i++)
            kids[i]->write_to_VBOs(endp, vpos, imat);
    }
    //searches for the correct animations as well as sets the correct element from the animation matrix array
    void set_animations(all_animations *all_anim,mat4 *matrices,int &animsize)
    {
        for (int ii = 0; ii < all_anim->animations.size(); ii++)
            if (all_anim->animations[ii].bone == name)
                animation.push_back(&all_anim->animations[ii]);

        mat = &matrices[index];
        animsize++;

        for (int i = 0; i < kids.size(); i++)
            kids[i]->set_animations(all_anim, matrices, animsize);
    }

    int getKeyFrameCount(std::string animationName) {
      for (auto anim : animation) {
          if (anim->name == animationName) {
              return anim->frames;
            }
          }
          return 1;
        }

        long long getDuration(std::string animationName) {
          for (auto anim : animation) {
            if (anim->name == animationName) {
              return anim->duration;
            }
          }
          return 0;
        }
};

int readtobone(string file,all_animations *all_animation, bone **proot);
