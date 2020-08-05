#include "pbd_system.hpp"
#include "constraints.hpp"
#include "point_cloud.hpp"

namespace pbd
{

PbdSystem::PbdSystem(int num_vertices)
{
  ds_ = std::make_unique<PointCloud>(num_vertices);
}

void PbdSystem::AddLengthConstraint(
    int idx1,
    int idx2,
    double target_len,
    int power)
{
  length_constraints_.push_back({idx1, idx2, target_len, power});
}

void PbdSystem::AddAngleConstraint(
    int idx1,
    int idx2,
    int idx3,
    double target_angle,
    double stiffness,
    int power)
{
  angle_constraints_.push_back(
      {idx1, idx2, idx3, target_angle, stiffness, power});
}

void PbdSystem::Integrate(double dt)
{
  ds_->Integrate(dt);
  HandleLengthConstraints(dt);
  HandleAngleConstraints(dt);
}

void PbdSystem::HandleLengthConstraints(double dt)
{
  for (const auto& c : length_constraints_)
  {
    for (int i = 0; i < c.power; ++i)
    {
      const auto p = ds_->GetPoint(c.idx1);
      const auto q = ds_->GetPoint(c.idx2);
      const auto pm = ds_->GetMass(c.idx1);
      const auto qm = ds_->GetMass(c.idx2);
      const auto target_len = c.target_len;
      glm::dvec2 dp, dq;
      GetLengthConstraintDelta(p,pm,q,qm,target_len,&dp,&dq);
      ds_->DisplacePoint(c.idx1, dp);
      ds_->AddVelocity(c.idx1, dp/dt);
      ds_->DisplacePoint(c.idx2, dq);
      ds_->AddVelocity(c.idx2, dq/dt);
    }
  }
}

void PbdSystem::HandleAngleConstraints(double dt)
{
  for (const auto& c : angle_constraints_)
  {
    for (int i = 0; i < c.power; ++i)
    {
      const auto p = ds_->GetPoint(c.idx1);
      const auto q = ds_->GetPoint(c.idx2);
      const auto r = ds_->GetPoint(c.idx3);
      const auto pm = ds_->GetMass(c.idx1);
      const auto qm = ds_->GetMass(c.idx2);
      const auto rm = ds_->GetMass(c.idx3);
      const auto target_angle = c.target_angle;
      const auto stiffness = c.stiffness;
      glm::dvec2 dp, dq, dr;
      GetAngleConstraintDelta(
          p,pm,q,qm,r,rm,
          target_angle,stiffness,
          &dp,&dq,&dr);
      ds_->DisplacePoint(c.idx1, dp);
      ds_->AddVelocity(c.idx1, dp/dt);
      ds_->DisplacePoint(c.idx2, dq);
      ds_->AddVelocity(c.idx2, dq/dt);
      ds_->DisplacePoint(c.idx3, dr);
      ds_->AddVelocity(c.idx3, dr/dt);
    }
  }
}

}