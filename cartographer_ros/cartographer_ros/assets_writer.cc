/*
 * Copyright 2016 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cartographer_ros/assets_writer.h"

#include "cartographer/common/make_unique.h"
#include "cartographer/io/null_points_processor.h"
#include "cartographer/io/ply_writing_points_processor.h"
#include "cartographer/io/points_processor.h"
#include "cartographer/io/xray_points_processor.h"

namespace cartographer_ros {

namespace carto = ::cartographer;

void WriteAssets(const std::vector<::cartographer::mapping::TrajectoryNode>&
                     trajectory_nodes,
                 const double voxel_size, const std::string& stem) {
  carto::io::NullPointsProcessor null_points_processor;
  carto::io::XRayPointsProcessor xy_xray_points_processor(
      voxel_size, carto::transform::Rigid3f::Rotation(
                      Eigen::AngleAxisf(-M_PI / 2.f, Eigen::Vector3f::UnitY())),
      stem + "_xray_xy.png", &null_points_processor);
  carto::io::XRayPointsProcessor yz_xray_points_processor(
      voxel_size, carto::transform::Rigid3f::Rotation(
                      Eigen::AngleAxisf(M_PI, Eigen::Vector3f::UnitZ())),
      stem + "_xray_yz.png", &xy_xray_points_processor);
  carto::io::XRayPointsProcessor xz_xray_points_processor(
      voxel_size, carto::transform::Rigid3f::Rotation(
                      Eigen::AngleAxisf(-M_PI / 2.f, Eigen::Vector3f::UnitZ())),
      stem + "_xray_xz.png", &yz_xray_points_processor);
  carto::io::PlyWritingPointsProcessor ply_writing_points_processor(
      stem + ".ply", &xz_xray_points_processor);

  for (const auto& node : trajectory_nodes) {
    const carto::sensor::LaserFan laser_fan = carto::sensor::TransformLaserFan(
        carto::sensor::Decompress(node.constant_data->laser_fan_3d),
        node.pose.cast<float>());

    auto points_batch = carto::common::make_unique<carto::io::PointsBatch>();
    points_batch->origin = laser_fan.origin;
    points_batch->points = laser_fan.returns;
    for (const uint8 reflectivity :
         node.constant_data->laser_fan_3d.reflectivities) {
      points_batch->colors.push_back(
          carto::io::Color{{reflectivity, reflectivity, reflectivity}});
    }
    ply_writing_points_processor.Process(std::move(points_batch));
  }
  ply_writing_points_processor.Flush();
}

}  // namespace cartographer_ros
