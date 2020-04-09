#include <vsg/all.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <cstddef>


#include <osg/Matrixd>
#include <osg/io_utils>

float computeDelta(const vsg::mat4& v, const osg::Matrixf& o)
{
    vsg::mat4::value_type delta = 0.0f;

    const vsg::mat4::value_type* view_ptr = v.data();
    const osg::Matrixf::value_type* osg_view_ptr = o.ptr();
    for(int i=0; i<16; ++i)
    {
        delta += fabs(view_ptr[i]-osg_view_ptr[i]);
    }
    return delta;
}

float computeDelta(const vsg::dmat4& v, const osg::Matrixd& o)
{
    vsg::dmat4::value_type delta = 0.0f;

    const vsg::dmat4::value_type* view_ptr = v.data();
    const osg::Matrixd::value_type* osg_view_ptr = o.ptr();
    for(int i=0; i<16; ++i)
    {
        delta += fabs(view_ptr[i]-osg_view_ptr[i]);
    }
    return delta;
}

int main(int /*argc*/, char** /*argv*/)
{

    vsg::vec2 v;

    v.x = 10.1f;
    v.y = 12.2f;


    std::cout<<"vec2(x="<<v.x<<", y="<<v.y<<")"<<std::endl;
    std::cout<<"vec2(r="<<v.r<<", g="<<v.g<<")"<<std::endl;
    std::cout<<"vec2(s="<<v.s<<", t="<<v.t<<")"<<std::endl;
    std::cout<<"vec2[0]=("<<v[0]<<", "<<v[1]<<")"<<std::endl;

    vsg::dvec3 n(2.0, 1.0, 0.5);
    std::cout<<"n(x="<<n.x<<", y="<<n.y<<", z="<<n.z<<")"<<std::endl;

    std::cout<<"n = "<<n<<std::endl;

    vsg::t_vec2<int> iv(2, 1);
    std::cout<<"iv = "<<iv<<std::endl;


    vsg::dvec4 colour(1.0, 0.9, 1.0, 0.5);
    std::cout<<"colour = ("<<colour<<")"<<std::endl;

    vsg::dmat4 mat;
    mat(3,0) = 102.3;
    std::cout<<"mat = "<<mat<<std::endl;

    vsg::t_mat4<short> cmat;
    std::cout<<"cmat = "<<cmat<<std::endl;
    std::cout<<"sizeof(cmat) = "<<sizeof(cmat)<<std::endl;

    vsg::ref_ptr<vsg::Object> object(new vsg::Object());
    object->setValue("matrix", mat);

    vsg::mat4 new_mat;
    if (object->getValue("matrix", new_mat))
    {
        std::cout<<"getValue(\"matrix\""<<new_mat<<std::endl;
    }


    vsg::mat4 proj = vsg::perspective(vsg::radians(45.0), 2.0, 0.1, 10.0);

    std::cout<<"proj = {"<<std::endl<<proj<<"}"<<std::endl;

    osg::Matrixf osg_proj = osg::Matrixf::perspective(45.0f, 2.0f, 0.1f, 10.0f);
    std::cout<<"osg_proj = "<<osg_proj<<std::endl;

    std::cout<<std::endl;

#if 1
    vsg::mat4 view = vsg::lookAt(vsg::vec3(2.0f, 2.0f, 2.0f), vsg::vec3(0.0f, 0.0f, 0.0f), vsg::vec3(0.0f, 0.0f, 1.0f));

    std::cout<<"view = {"<<std::endl<<view<<"}"<<std::endl;

    osg::Matrixf osg_view = osg::Matrixf::lookAt(osg::Vec3(2.0f,2.0f,2.0f), osg::Vec3(0.0f,0.0f, 0.0f), osg::Vec3(0.0f,0.0f,1.0f));
    std::cout<<"osg_view = "<<osg_view<<std::endl;
#else
    vsg::mat4 view = vsg::lookAt(vsg::vec3(0,0,0), vsg::vec3(0,10,0), vsg::vec3(0,0,1));

    std::cout<<"view = {"<<std::endl<<view<<"}"<<std::endl;

    osg::Matrixf osg_view = osg::Matrixf::lookAt(osg::Vec3(0,0,0), osg::Vec3(0,10, 0), osg::Vec3(0,0,1));
    std::cout<<"osg_view = "<<osg_view<<std::endl;
#endif

    float* view_ptr = view.data();
    float* osg_view_ptr = osg_view.ptr();
    for(int i=0; i<16; ++i)
    {
        std::cout<<" view_ptr["<<i<<"]="<<view_ptr[i]<<"   osg_voew_ptr["<<i<<"]="<<osg_view_ptr[i]<<std::endl;
    }

    std::cout<<"delta for proj "<<computeDelta(proj, osg_proj)<<std::endl;
    std::cout<<"delta for view "<<computeDelta(view, osg_view)<<std::endl;

    vsg::mat4 rot = vsg::rotate(vsg::radians(45.0f), 0.0f, 0.0f, 1.0f);
    osg::Matrixf osg_rot = osg::Matrixf::rotate(vsg::radians(45.0f), 0.0f, 0.0f, 1.0f);

    std::cout<<"delta for rot "<<computeDelta(rot, osg_rot)<<std::endl;
    std::cout<<"rot = {"<<rot<<"}"<<std::endl;
    std::cout<<"osg_rot = {"<<osg_rot<<"}"<<std::endl;


    constexpr vsg::mat4 trans = vsg::translate(vsg::vec3(1.0f, 2.0f, 3.0f));
    osg::Matrixf osg_trans = osg::Matrixf::translate(1.0f, 2.0f, 3.0f);

    std::cout<<"delta for trans "<<computeDelta(trans, osg_trans)<<std::endl;
    std::cout<<"trans = {"<<trans<<"}"<<std::endl;
    std::cout<<"osg_trans = {"<<osg_trans<<"}"<<std::endl;


    constexpr vsg::mat4 scale = vsg::scale(vsg::vec3(1.0f, 2.0f, 3.0f));
    osg::Matrixf osg_scale = osg::Matrixf::scale(1.0f, 2.0f, 3.0f);

    std::cout<<"delta for scale "<<computeDelta(scale, osg_scale)<<std::endl;
    std::cout<<"scale = {"<<scale<<"}"<<std::endl;
    std::cout<<"osg_scale = {"<<osg_scale<<"}"<<std::endl;

    // note VSG and OSG multiplication order reversed.
    vsg::mat4 result = scale*trans*rot;
    osg::Matrixf osg_result = osg_rot*osg_trans*osg_scale;

    std::cout<<"delta for result "<<computeDelta(result, osg_result)<<std::endl;
    std::cout<<"result = {"<<result<<"}"<<std::endl;
    std::cout<<"osg_result = {"<<osg_result<<"}"<<std::endl;


    return 0;
}
