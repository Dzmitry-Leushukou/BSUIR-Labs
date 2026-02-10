-- Add support for multiple photos in trip completions
-- Create a junction table to connect trip completions with multiple photos

-- Create trip_completion_photos table
CREATE TABLE IF NOT EXISTS trip_completion_photos (
    id SERIAL PRIMARY KEY,
    trip_completion_id INT NOT NULL REFERENCES trip_completions(id) ON DELETE CASCADE,
    photo_id INT NOT NULL REFERENCES photos(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(trip_completion_id, photo_id)
);

-- Migrate existing single photo data to the new table
INSERT INTO trip_completion_photos (trip_completion_id, photo_id)
SELECT id, completion_photo_id
FROM trip_completions
WHERE completion_photo_id IS NOT NULL;

-- Add a column to track the primary photo for each trip completion
ALTER TABLE trip_completion_photos ADD COLUMN is_primary BOOLEAN DEFAULT FALSE;

-- Set the migrated photo as primary
UPDATE trip_completion_photos 
SET is_primary = TRUE 
WHERE (trip_completion_id, photo_id) IN (
    SELECT id, completion_photo_id 
    FROM trip_completions 
    WHERE completion_photo_id IS NOT NULL
);

-- Temporarily store the original completion_photo_id values
CREATE TEMP TABLE temp_trip_completion_photos AS 
SELECT id, completion_photo_id 
FROM trip_completions 
WHERE completion_photo_id IS NOT NULL;

-- Remove the completion_photo_id column from trip_completions
ALTER TABLE trip_completions DROP COLUMN completion_photo_id;

-- Create indexes for better performance
CREATE INDEX IF NOT EXISTS idx_trip_completion_photos_trip_completion_id ON trip_completion_photos(trip_completion_id);
CREATE INDEX IF NOT EXISTS idx_trip_completion_photos_photo_id ON trip_completion_photos(photo_id);
CREATE INDEX IF NOT EXISTS idx_trip_completion_photos_is_primary ON trip_completion_photos(is_primary);