#pragma once

#include <string_view>

namespace lobby {
/**
 * @brief Queue loading of Custom Maps for the current private match guest
 */
void prepare_guest_usermap();

/**
 * @brief Offer to download a missing Workshop map through BOIII
 * @param id A numeric Workshop ID, optionally prefixed with "usermaps/"
 * @return True for a valid numeric ID, even if no new prompt is shown
 */
bool offer_missing_guest_usermap(std::string_view id);

/**
 * @brief Refresh the preview after reloading the local map pool
 *
 * Updates the image if the offered map is installed and still selected
 * in the same host's private match
 */
void refresh_downloaded_usermap_preview();
} // namespace lobby
