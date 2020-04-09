#include <vsg/all.h>

#include <iostream>
#include <vector>
#include <chrono>

struct PrintVisitor : public vsg::Visitor
{
    using Visitor::apply;

    void apply(vsg::Object& object) override
    {
        std::cout<<"apply(vsg::Object& "<<&object<<")"<<std::endl;
        object.traverse(*this);
    }

    void apply(vsg::Node& node) override
    {
        std::cout<<"apply(vsg::Node& "<<&node<<")"<<std::endl;
        node.traverse(*this);
    }

    void apply(vsg::Group& group) override
    {
        std::cout<<"apply(vsg::Group& "<<&group<<") getNumChildren()="<<group.getNumChildren()<<std::endl;
        group.traverse(*this);
    }

    void apply(vsg::QuadGroup& group) override
    {
        std::cout<<"apply(vsg::QuadGroup& "<<&group<<") getNumChildren()="<<group.getNumChildren()<<std::endl;
        group.traverse(*this);
    }

    void apply(vsg::LOD& lod) override
    {
        std::cout<<"apply(vsg::LOD& "<<&lod<<") getNumChildren()="<<lod.getNumChildren()<<std::endl;
        lod.traverse(*this);
    }
};

int main(int /*argc*/, char** /*argv*/)
{

    auto group = vsg::Group::create();

    // set up LOD
    auto lod = vsg::LOD::create();
    lod->addChild(vsg::LOD::LODChild{0.0, vsg::Node::create()});
    lod->addChild(vsg::LOD::LODChild{0.5, vsg::Node::create()});
    PrintVisitor visitor;
    group->accept(visitor);


    return 0;
}
