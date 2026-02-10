-- Fix status column length in rentals table
-- This addresses the "value too long for type character varying(10)" error
-- The status column needs to accommodate values like 'pending_completion' (18 chars)

ALTER TABLE rentals ALTER COLUMN status TYPE VARCHAR(20);

-- Also make sure the check constraint is properly defined to include allowed values
ALTER TABLE rentals DROP CONSTRAINT IF EXISTS rentals_status_check;
ALTER TABLE rentals ADD CONSTRAINT rentals_status_check CHECK (status IN ('active','completed','cancelled','pending_completion'));