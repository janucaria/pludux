#include <algorithm>
#include <vector>

#include <pludux/asset.hpp>

#include <pludux/asset_data_provider.hpp>

namespace pludux {

AssetDataProvider::AssetDataProvider(const Asset& asset)
: asset_{asset}
{
}

} // namespace pludux
