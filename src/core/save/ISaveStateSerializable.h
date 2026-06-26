#pragma once

class SaveStateWriter;
class SaveStateReader;

class ISaveStateSerializable
{
public:
    virtual ~ISaveStateSerializable() = default;

    virtual void serialize(SaveStateWriter& writer) const = 0;
    virtual void deserialize(SaveStateReader& reader) = 0;
};
