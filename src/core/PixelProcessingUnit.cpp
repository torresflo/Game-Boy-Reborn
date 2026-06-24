#include "PixelProcessingUnit.h"

#include <format>
#include <algorithm>

#include "CentralProcessingUnit.h"
#include "MathUtils.h"
#include "MemoryBus.h"

namespace
{
    constexpr u16 TileDataStartAddress = 0x8000;

    //The fetcher runs ahead of the pixel pusher (FIFO look-ahead), so window tiles must
    //keep being fetched a bit past the last visible column to cover the final pushed pixels
    constexpr u32 WindowFetcherLookaheadPixels = 14;
}

void PixelProcessingUnit::initialize(MemoryBus* bus, CentralProcessingUnit* cpuPtr)
{
    memoryBus = bus;
    cpu = cpuPtr;

    currentFrame = 0;
    lineTicks = 0;
    frameBuffer.fill(0);

    LCD.initialize();
    setLCDMode(LCDMode::ObjectAccessMemoryScan);

    pixelFIFOContext.initialize();

    lineObjects.reserve(MaxObjects);
    fetchedObjects.reserve(MaxObjectsPerFetch);

    windowLine = 0;
}

void PixelProcessingUnit::tick()
{
    lineTicks++;
    LCDMode currentMode = getLCDMode();
    switch(currentMode)
    {
        case LCDMode::HorizontalBlank:
            updateHorizontalBlankMode();
            break;
        case LCDMode::VerticalBlank:
            updateVerticalBlankMode();
            break;
        case LCDMode::ObjectAccessMemoryScan:
            updateObjectAccessMemoryScanMode();
            break;
        case LCDMode::PixelDrawing:
            updatePixelDrawingMode();
            break;
    }
}

u8 PixelProcessingUnit::readRegister(u16 address) const
{
    switch(address)
    {
        case 0xFF40:
            return LCD.control;
        case 0xFF41:
            return LCD.status;
        case 0xFF42:
            return LCD.scrollY;
        case 0xFF43:
            return LCD.scrollX;
        case 0xFF44:
            return LCD.coordinateY;
        case 0xFF45:
            return LCD.compareY;
        case 0xFF47:
            return LCD.backgroundPalette;
        case 0xFF48:
            return LCD.objectPaletteData[0];
        case 0xFF49:
            return LCD.objectPaletteData[1];
        case 0xFF4A:
            return LCD.windowY;
        case 0xFF4B:
            return LCD.windowX;
        default:
            Log::print(LogLevel::Error, std::format("Unsupported PPU register reading (0x{:4X}).", address));
            return 0xFF;
    }
}

void PixelProcessingUnit::writeRegister(u16 address, u8 value)
{
    switch(address)
    {
        case 0xFF40:
            LCD.control = value;
            break;
        case 0xFF41:
            setHorizontalBlankInterruptEnabled(MathUtils<u8>::getBitValue(value, 3));
            setVerticalBlankInterruptEnabled(MathUtils<u8>::getBitValue(value, 4));
            setObjectAccessMemoryInterruptEnabled(MathUtils<u8>::getBitValue(value, 5));
            setLYCInterruptEnabled(MathUtils<u8>::getBitValue(value, 6));
            break;
        case 0xFF42:
            LCD.scrollY = value;
            break;
        case 0xFF43:
            LCD.scrollX = value;
            break;
        case 0xFF44: //LY is read-only -> any write resets it
            LCD.coordinateY = 0;
            break;
        case 0xFF45:
            LCD.compareY = value;
            break;
        case 0xFF47:
            LCD.updatePaletteData(value, 0);
            break;
        case 0xFF48:
            LCD.updatePaletteData(value & 0b11111100, 1);
            break;
        case 0xFF49:
            LCD.updatePaletteData(value & 0b11111100, 2);
            break;
        case 0xFF4A:
            LCD.windowY = value;
            break;
        case 0xFF4B:
            LCD.windowX = value;
            break;
        default:
            Log::print(LogLevel::Error, std::format("Unsupported PPU register writing (0x{:4X}).", address));
            break;
    }
}

bool PixelProcessingUnit::getBackgroundEnabled() const
{
    return MathUtils<u8>::getBitValue(LCD.control, 0);
}

bool PixelProcessingUnit::getObjectEnabled() const
{
    return MathUtils<u8>::getBitValue(LCD.control, 1);
}

SpriteSize PixelProcessingUnit::getObjectSize() const
{
    return MathUtils<u8>::getBitValue(LCD.control, 2) ? SpriteSize::EightBySixteen : SpriteSize::EightByEight;
}

TileMapArea PixelProcessingUnit::getBackgroundTileMapArea() const
{
    return MathUtils<u8>::getBitValue(LCD.control, 3) ? TileMapArea::High : TileMapArea::Low;
}

TileDataArea PixelProcessingUnit::getTileDataArea() const
{
    return MathUtils<u8>::getBitValue(LCD.control, 4) ? TileDataArea::Unsigned : TileDataArea::Signed;
}

bool PixelProcessingUnit::getWindowEnabled() const
{
    return MathUtils<u8>::getBitValue(LCD.control, 5);
}

TileMapArea PixelProcessingUnit::getWindowTileMapArea() const
{
    return MathUtils<u8>::getBitValue(LCD.control, 6) ? TileMapArea::High : TileMapArea::Low;
}

bool PixelProcessingUnit::getLCDEnabled() const
{
    return MathUtils<u8>::getBitValue(LCD.control, 7);
}

LCDMode PixelProcessingUnit::getLCDMode() const
{
    u8 modeValue = (MathUtils<u8>::getBitValue(LCD.status, 1) ? 2 : 0)
                        | (MathUtils<u8>::getBitValue(LCD.status, 0) ? 1 : 0);
    return static_cast<LCDMode>(modeValue);
}

void PixelProcessingUnit::setLCDMode(LCDMode mode)
{
    u8 modeValue = static_cast<u8>(mode);
    MathUtils<u8>::setBitValue(LCD.status, 0, MathUtils<u8>::getBitValue(modeValue, 0));
    MathUtils<u8>::setBitValue(LCD.status, 1, MathUtils<u8>::getBitValue(modeValue, 1));
}

bool PixelProcessingUnit::getCoincidenceFlag() const
{
    return MathUtils<u8>::getBitValue(LCD.status, 2);
}

void PixelProcessingUnit::setCoincidenceFlag(bool flag)
{
    MathUtils<u8>::setBitValue(LCD.status, 2, flag);
}

bool PixelProcessingUnit::getHorizontalBlankInterruptEnabled() const
{
    return MathUtils<u8>::getBitValue(LCD.status, 3);
}

void PixelProcessingUnit::setHorizontalBlankInterruptEnabled(bool enabled)
{
    MathUtils<u8>::setBitValue(LCD.status, 3, enabled);
}

bool PixelProcessingUnit::getVerticalBlankInterruptEnabled() const
{
    return MathUtils<u8>::getBitValue(LCD.status, 4);
}

void PixelProcessingUnit::setVerticalBlankInterruptEnabled(bool enabled)
{
    MathUtils<u8>::setBitValue(LCD.status, 4, enabled);
}

bool PixelProcessingUnit::getObjectAccessMemoryInterruptEnabled() const
{
    return MathUtils<u8>::getBitValue(LCD.status, 5);
}

void PixelProcessingUnit::setObjectAccessMemoryInterruptEnabled(bool enabled)
{
    MathUtils<u8>::setBitValue(LCD.status, 5, enabled);
}

bool PixelProcessingUnit::getLYCInterruptEnabled() const
{
    return MathUtils<u8>::getBitValue(LCD.status, 6);
}

bool PixelProcessingUnit::isWindowVisible() const
{
    return getWindowEnabled()
        && LCD.windowX <= ScreenWidth + 6 //166
        && LCD.windowY < ScreenHeight;
}

bool PixelProcessingUnit::isWindowActiveOnLine() const
{
    return isWindowVisible() && LCD.coordinateY >= LCD.windowY;
}

void PixelProcessingUnit::setLYCInterruptEnabled(bool enabled)
{
    MathUtils<u8>::setBitValue(LCD.status, 6, enabled);
}

const std::array<u32, PixelProcessingUnit::ScreenWidth * PixelProcessingUnit::ScreenHeight>& PixelProcessingUnit::getFrameBuffer() const
{
    return frameBuffer;
}

const std::array<u32, 4>& PixelProcessingUnit::getObjectColors(u8 paletteNumber) const
{
    return static_cast<bool>(paletteNumber) ? LCD.object2Colors : LCD.object1Colors;
}

PixelProcessingUnit::Tile PixelProcessingUnit::decodeTileAtAddress(u16 tileAddress) const
{
    Tile tile{};
    for(u32 row = 0; row < TileSize; ++row)
    {
        u8 lowByte = memoryBus->read(tileAddress + static_cast<u16>(row * 2));
        u8 highByte = memoryBus->read(tileAddress + static_cast<u16>(row * 2 + 1));

        for(u32 column = 0; column < TileSize; ++column)
        {
            u32 bit = 7 - column;
            tile[row * TileSize + column] = (MathUtils<u8>::getBitValue(highByte, bit) ? 2 : 0)
                                                    | (MathUtils<u8>::getBitValue(lowByte, bit) ? 1 : 0);
        }
    }
    return tile;
}

PixelProcessingUnit::Tile PixelProcessingUnit::decodeTileAIndex(u32 tileIndex) const
{
    u16 tileAddress = TileDataStartAddress + static_cast<u16>(tileIndex * PixelProcessingUnit::BytesPerTile);
    return decodeTileAtAddress(tileAddress);
}

void PixelProcessingUnit::updateHorizontalBlankMode()
{
    if(lineTicks >= TicksPerLine)
    {
        incrementCoordinateY();
        if(LCD.coordinateY >= ScreenHeight)
        {
            setLCDMode(LCDMode::VerticalBlank);
            cpu->requestInterrupt(InterruptType::VBlank);

            if(getVerticalBlankInterruptEnabled())
                cpu->requestInterrupt(InterruptType::LCD);

            currentFrame++;
        }
        else
        {
            setLCDMode(LCDMode::ObjectAccessMemoryScan);
        }

        lineTicks = 0;
    }
}

void PixelProcessingUnit::updateVerticalBlankMode()
{
    if(lineTicks >= TicksPerLine)
    {
        incrementCoordinateY();
        if(LCD.coordinateY >= LinesPerFrame)
        {
            setLCDMode(LCDMode::ObjectAccessMemoryScan);
            resetCoordinateY();
        }

        lineTicks = 0;
    }
}

void PixelProcessingUnit::updateObjectAccessMemoryScanMode()
{
    if(lineTicks >= 80)
    {
        setLCDMode(LCDMode::PixelDrawing);
        pixelFIFOContext.state = PixelFIFOState::GetTile;
        pixelFIFOContext.lineX = 0;
        pixelFIFOContext.fetchX = 0;
        pixelFIFOContext.pushedX = 0;
        pixelFIFOContext.fifoX = 0;
    }

    //Real hardware scans OAM progressively across all 80 ticks (2 entries/tick); we load every entry up front for simplicity
    if(lineTicks == 1)
        loadLineObjects();
}

void PixelProcessingUnit::updatePixelDrawingMode()
{
    processPixelFIFO();

    if(pixelFIFOContext.pushedX >= ScreenWidth)
    {
        resetPixelFIFO();
        setLCDMode(LCDMode::HorizontalBlank);

        if(getHorizontalBlankInterruptEnabled())
            cpu->requestInterrupt(InterruptType::LCD);
    }
}

void PixelProcessingUnit::incrementCoordinateY()
{
    if(isWindowActiveOnLine())
        windowLine++;

    LCD.coordinateY++;
    bool matches = (LCD.coordinateY == LCD.compareY);
    setCoincidenceFlag(matches);
    if(matches)
    {
        if(getLYCInterruptEnabled())
            cpu->requestInterrupt(InterruptType::LCD);
    }
}

void PixelProcessingUnit::resetCoordinateY()
{
    LCD.coordinateY = 0;
    windowLine = 0;
}

void PixelProcessingUnit::processPixelFIFO()
{
    pixelFIFOContext.mapY = LCD.coordinateY + LCD.scrollY;
    pixelFIFOContext.mapX = pixelFIFOContext.fetchX + LCD.scrollX;
    pixelFIFOContext.tileY = ((LCD.coordinateY + LCD.scrollY) % TileSize) * 2;

    if(!MathUtils<u32>::getBitValue(lineTicks, 0)) //Fetcher advances every other tick
        updateFetchPixel();

    pushPixelInBuffer();    
}

void PixelProcessingUnit::updateFetchPixel()
{
    switch(pixelFIFOContext.state)
    {
        case PixelFIFOState::GetTile:
        {
            fetchedObjects.clear();

            if(getBackgroundEnabled())
            {
                TileMapArea mapArea = getBackgroundTileMapArea();
                u16 address = static_cast<u16>(mapArea) + (pixelFIFOContext.mapX / TileSize) + ((pixelFIFOContext.mapY / TileSize) * 32);
                pixelFIFOContext.backgroundFetchData[0] = memoryBus->read(address);

                TileDataArea dataArea = getTileDataArea();
                if(dataArea == TileDataArea::Signed)
                    pixelFIFOContext.backgroundFetchData[0] += 128;

                loadWindowTile();
            }

            if(getObjectEnabled() && !lineObjects.empty())
                selectFetchedObjects();

            pixelFIFOContext.state = PixelFIFOState::GetTileDataLow;
            pixelFIFOContext.fetchX += 8;
            break;
        }
        case PixelFIFOState::GetTileDataLow:
        {
            TileDataArea dataArea = getTileDataArea();
            u16 address = static_cast<u16>(dataArea) + (pixelFIFOContext.backgroundFetchData[0] * BytesPerTile) + pixelFIFOContext.tileY;
            pixelFIFOContext.backgroundFetchData[1] = memoryBus->read(address);

            loadObjectData(0);

            pixelFIFOContext.state = PixelFIFOState::GetTileDataHigh;
            break;
        }
        case PixelFIFOState::GetTileDataHigh:
        {
            TileDataArea dataArea = getTileDataArea();
            u16 address = static_cast<u16>(dataArea) + (pixelFIFOContext.backgroundFetchData[0] * BytesPerTile) + pixelFIFOContext.tileY + 1;
            pixelFIFOContext.backgroundFetchData[2] = memoryBus->read(address);
            
            loadObjectData(1);

            pixelFIFOContext.state = PixelFIFOState::Sleep;
            break;
        }
        case PixelFIFOState::Sleep:
        {
            pixelFIFOContext.state = PixelFIFOState::Push;
            break;
        }
        case PixelFIFOState::Push:
        {
            if(addFetchedDataInPixelFIFO())
                pixelFIFOContext.state = PixelFIFOState::GetTile;
            break;
        }
    }
}

void PixelProcessingUnit::pushPixelInBuffer()
{
    if(pixelFIFOContext.queue.size() > 8)
    {
        u32 pixelData = pixelFIFOContext.queue.front();
        pixelFIFOContext.queue.pop();

        if(pixelFIFOContext.lineX >= LCD.scrollX % 8)
        {
            u32 position = pixelFIFOContext.pushedX + (LCD.coordinateY * ScreenWidth);
            frameBuffer[position] = pixelData;
            pixelFIFOContext.pushedX++;
        }

        pixelFIFOContext.lineX++;
    }
}

bool PixelProcessingUnit::addFetchedDataInPixelFIFO()
{
    if(pixelFIFOContext.queue.size() > 8)
    {
        //Queue is full
        return false;
    }

    s32 x = pixelFIFOContext.fetchX - (8 - (LCD.scrollX % 8));

    for(u8 i = 0; i < 8; ++i)
    {
        u8 bitPosition = 7 - i;
        u8 low = MathUtils<u8>::getBitValue(pixelFIFOContext.backgroundFetchData[1], bitPosition);
        u8 high = MathUtils<u8>::getBitValue(pixelFIFOContext.backgroundFetchData[2], bitPosition);
        u8 backgroundColorIndex = getBackgroundEnabled() ? (low | (high << 1)) : 0;
        u32 color = LCD.backgroundColors[backgroundColorIndex];

        if(getObjectEnabled())
            color = fetchObjectPixel(color, backgroundColorIndex);

        if(x >= 0)
        {
            pixelFIFOContext.queue.push(color);
            pixelFIFOContext.fifoX++;
        }
    }

    return true;
}

void PixelProcessingUnit::resetPixelFIFO()
{
    pixelFIFOContext.queue = std::queue<u32>();
}

s32 PixelProcessingUnit::getObjectFIFOX(const ObjectAttributeMemoryEntry& object) const
{
    return (object.x - 8) + (LCD.scrollX % 8);
}

void PixelProcessingUnit::selectFetchedObjects()
{
    for(const ObjectAttributeMemoryEntry& object : lineObjects)
    {
        s32 objectX = getObjectFIFOX(object);
        bool isInFetchWindow = objectX >= pixelFIFOContext.fetchX - 8 && objectX < pixelFIFOContext.fetchX + 8;

        if(isInFetchWindow)
            fetchedObjects.push_back(object);

        if(fetchedObjects.size() >= MaxObjectsPerFetch)
            break;
    }
}

void PixelProcessingUnit::loadObjectData(u8 offset)
{
    u8 currentY = LCD.coordinateY;
    u8 objectHeight = static_cast<u8>(getObjectSize());

    for(u32 i = 0; i < fetchedObjects.size(); ++i)
    {
        const ObjectAttributeMemoryEntry& object = fetchedObjects[i];
        u8 tileY = ((currentY + ObjectScreenYOffset) - object.y) * 2;
        if(object.yFlip) //flipped upside down
            tileY = ((objectHeight * 2) - 2) - tileY;

        u8 tileIndex = object.tileIndex;
        if(objectHeight == static_cast<u8>(SpriteSize::EightBySixteen))
            MathUtils<u8>::setBitValue(tileIndex, 0, false); //Lower bit of tile index is ignored for 8x16 objects

        u16 address = TileDataStartAddress + (tileIndex * BytesPerTile) + tileY + offset;
        pixelFIFOContext.fetchEntryData[(i * 2) + offset] = memoryBus->read(address);
    }
}

void PixelProcessingUnit::loadLineObjects()
{
    u8 currentY = LCD.coordinateY;
    u8 objectHeight = static_cast<u8>(getObjectSize());
    lineObjects.clear();

    u32 matchedObjectCount = 0;
    for(u8 i = 0; i < MemoryBus::OAMEntries && matchedObjectCount < MaxObjects; ++i)
    {
        ObjectAttributeMemoryEntry entry = memoryBus->readObject(i); //Direct access by index

        bool isOnCurrentLine = entry.y <= currentY + ObjectScreenYOffset && entry.y + objectHeight > currentY + ObjectScreenYOffset;
        if(!isOnCurrentLine)
            continue;

        matchedObjectCount++; //Counts towards the per-line cap even if not visible (x == 0)

        if(entry.x != 0) //x == 0 is fully off-screen, nothing to render
            lineObjects.push_back(entry);
    }

    //Smaller X is drawn on top; ties keep OAM order (stable_sort preserves it)
    std::stable_sort(lineObjects.begin(), lineObjects.end(), [](const ObjectAttributeMemoryEntry& a, const ObjectAttributeMemoryEntry& b)
    {
        return a.x < b.x;
    });
}

u32 PixelProcessingUnit::fetchObjectPixel(u32 currentColor, u8 backgroundColorIndex)
{
    for(u8 i = 0; i < fetchedObjects.size(); ++i)
    {
        const ObjectAttributeMemoryEntry& object = fetchedObjects[i];
        s32 objectX = getObjectFIFOX(object);

        s32 offset = pixelFIFOContext.fifoX - objectX;
        if(offset < 0 || offset > 7) //Out of bounds
            continue;

        u8 bitPosition = static_cast<u8>(7 - offset);
        if(object.xFlip)
            bitPosition = static_cast<u8>(offset);

        u8 low = MathUtils<u8>::getBitValue(pixelFIFOContext.fetchEntryData[i * 2], bitPosition);
        u8 high = MathUtils<u8>::getBitValue(pixelFIFOContext.fetchEntryData[(i * 2) + 1], bitPosition);
        u8 paletteIndex = low | (high << 1);

        if(paletteIndex == 0) //Transparent, let a lower-priority object show through
            continue;

        //Highest-priority opaque object at this pixel: it alone decides object-vs-background, win or lose
        bool backgroundWins = object.backgroundPriority && backgroundColorIndex != 0;
        if(!backgroundWins)
            currentColor = object.paletteNumber ? LCD.object2Colors[paletteIndex] : LCD.object1Colors[paletteIndex];

        break;
    }
    return currentColor;
}

void PixelProcessingUnit::loadWindowTile()
{
    if(!isWindowActiveOnLine())
        return;

    bool fetcherReachedWindowColumn = pixelFIFOContext.fetchX + 7u >= LCD.windowX
        && pixelFIFOContext.fetchX + 7u < LCD.windowX + ScreenWidth + WindowFetcherLookaheadPixels;
    if(!fetcherReachedWindowColumn)
        return;

    u8 tileRow = windowLine / TileSize;

    TileMapArea tileMapArea = getWindowTileMapArea();
    u16 address = static_cast<u16>(tileMapArea) + ((pixelFIFOContext.fetchX + 7u - LCD.windowX) / TileSize) + (tileRow * 32);
    pixelFIFOContext.backgroundFetchData[0] = memoryBus->read(address);

    TileDataArea dataArea = getTileDataArea();
    if(dataArea == TileDataArea::Signed)
        pixelFIFOContext.backgroundFetchData[0] += 128;
}
