/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cstdio>
#include <memory>
#include "ImageSource.h"
#include "src/Logger.h"

namespace maps {

ImageSource::ImageSource(std::shared_ptr<img::Image> image):
    image(image)
{
}

int ImageSource::getMinZoomLevel() {
    double maxDim = std::max(image->getWidth(), image->getHeight());
    double minN = std::log2(maxDim / TILE_SIZE);

    return -minN;
}

int ImageSource::getMaxZoomLevel() {
    return 0;
}

int ImageSource::getInitialZoomLevel() {
    return -1;
}

img::Point<double> ImageSource::suggestInitialCenter() {
    int fullWidth = image->getWidth();
    int fullHeight = image->getHeight();
    auto scale = zoomToScale(getInitialZoomLevel());
    return img::Point<double>{fullWidth / 2.0 / TILE_SIZE * scale, fullHeight / 6.0 / TILE_SIZE * scale};
}

float ImageSource::zoomToScale(int zoom) {
    if (zoom < 0) {
        return 1.0 / (1 << -zoom);
    } else {
        return 1.0 * (1 << zoom);
    }
}

bool ImageSource::supportsWorldCoords() {
    return false;
}

img::Point<int> ImageSource::getTileDimensions(int zoom) {
    return img::Point<int>{TILE_SIZE, TILE_SIZE};
}

img::Point<double> ImageSource::transformZoomedPoint(double oldX, double oldY, int oldZoom, int newZoom) {
    int fullWidth = image->getWidth();
    int fullHeight = image->getHeight();

    double oldWidth = fullWidth * zoomToScale(oldZoom);
    double newWidth = fullWidth * zoomToScale(newZoom);
    double oldHeight = fullHeight * zoomToScale(oldZoom);
    double newHeight = fullHeight * zoomToScale(newZoom);

    double x = oldX / oldWidth * newWidth;
    double y = oldY / oldHeight * newHeight;

    return img::Point<double>{x, y};
}

bool ImageSource::checkAndCorrectTileCoordinates(int& x, int& y, int zoom) {
    int fullWidth = image->getWidth();
    int fullHeight = image->getHeight();
    auto scale = zoomToScale(zoom);
    if (x < 0 || x * TILE_SIZE >= fullWidth * scale || y < 0 || y * TILE_SIZE >= fullHeight * scale) {
        return false;
    }

    return true;
}

std::string ImageSource::getUniqueTileName(int x, int y, int zoom) {
    if (!checkAndCorrectTileCoordinates(x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y;
    return nameStream.str();
}

std::unique_ptr<img::Image> ImageSource::loadTileImage(int x, int y, int zoom) {
    auto scale = zoomToScale(zoom);
    auto tile = std::make_unique<img::Image>(TILE_SIZE / scale, TILE_SIZE / scale, 0);
    image->copyTo(*tile, x * TILE_SIZE / scale, y * TILE_SIZE / scale);
    tile->scale(TILE_SIZE, TILE_SIZE);

    return tile;
}

void ImageSource::cancelPendingLoads() {
}

void ImageSource::resumeLoading() {
}

img::Point<double> ImageSource::worldToXY(double lon, double lat, int zoom) {
    return img::Point<double>{lon, lat};
}

img::Point<double> ImageSource::xyToWorld(double x, double y, int zoom) {
    return img::Point<double>{x, y};
}

} /* namespace maps */