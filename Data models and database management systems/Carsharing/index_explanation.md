# Database Indexes Explanation

This document explains the necessity and usage of indexes in the carsharing database.

## What are Database Indexes?

Database indexes are data structures that improve the speed of data retrieval operations on a database table at the cost of additional writes and storage space. They are similar to an index in a book - they allow the database to find data quickly without scanning the entire table.

## Indexes Used in the Carsharing Database

### Primary Indexes
- **Primary Key Indexes**: Automatically created for primary key columns (e.g., `id` in all tables)
- **Unique Indexes**: Automatically created for unique constraints (e.g., `email` in users, `vin` in cars)

### Additional Indexes in Our Schema

1. **Users Table**
   - `idx_users_email`: Index on `email` column for fast user lookup during authentication
   - `idx_users_role_id`: Index on `role_id` for efficient role-based queries

2. **Cars Table**
   - `idx_cars_vin`: Index on `vin` for fast car identification
   - `idx_cars_plate_number`: Index on `plate_number` for quick plate-based searches
   - `idx_cars_status`: Index on `status` for filtering cars by availability

3. **Rentals Table**
   - `idx_rentals_user_id`: Index on `user_id` for finding user's rentals
   - `idx_rentals_car_id`: Index on `car_id` for finding rentals for a specific car
   - `idx_rentals_status`: Index on `status` for filtering rentals by status
   - `idx_rentals_started_at`: Index on `started_at` for date range queries
   - `idx_rentals_ended_at`: Index on `ended_at` for date range queries

4. **Maintenance Requests Table**
   - `idx_maintenance_requests_car_id`: Index on `car_id` for finding maintenance requests for specific cars
   - `idx_maintenance_requests_status`: Index on `status` for filtering requests by status

5. **Payment Logs Table**
   - `idx_payment_logs_rental_id`: Index on `rental_id` for finding payments for specific rentals
   - `idx_payment_logs_user_id`: Index on `user_id` for finding payments by specific users

6. **Photos Table**
   - `idx_photos_user_id`: Index on `user_id` for finding photos related to specific users
   - `idx_photos_car_id`: Index on `car_id` for finding photos related to specific cars
   - `idx_photos_uploaded_by`: Index on `uploaded_by` for finding photos uploaded by specific users

7. **Sessions Table**
   - `idx_sessions_user_id`: Index on `user_id` for finding sessions for specific users

8. **Car States Table**
   - `idx_car_states_car_id`: Index on `car_id` for finding states of specific cars
   - `idx_car_states_checked_by`: Index on `checked_by` for finding inspections done by specific users

9. **Action Logs Table**
   - `idx_action_logs_actor_user_id`: Index on `actor_user_id` for finding actions by specific users
   - `idx_action_logs_action_type`: Index on `action_type` for filtering logs by action type
   - `idx_action_logs_created_at`: Index on `created_at` for time-based queries
   - `idx_action_logs_target_user_id`: Index on `target_user_id` for finding actions targeting specific users

## Why Indexes Are Necessary

1. **Performance Improvement**: Indexes dramatically speed up SELECT queries, especially on large datasets. Without indexes, the database would need to perform a full table scan for each query.

2. **Query Optimization**: For a carsharing application with potentially millions of records in tables like rentals, users, and cars, indexes ensure that common queries execute quickly.

3. **Search Efficiency**: In our application, users will frequently search for available cars, check rental history, or filter by status. Indexes make these operations efficient.

4. **Join Performance**: Our schema has many foreign key relationships. Indexes on these foreign key columns improve the performance of JOIN operations.

5. **Time-based Queries**: Many operations in our system involve time ranges (rental periods, maintenance schedules, etc.). Indexes on timestamp columns make these queries efficient.

6. **User Authentication**: The application will frequently look up users by email during authentication. An index on the email column ensures fast login operations.

## Trade-offs of Using Indexes

1. **Storage Space**: Each index requires additional disk space.
2. **Write Performance**: INSERT, UPDATE, and DELETE operations become slower as indexes also need to be updated.
3. **Maintenance Overhead**: The database needs to maintain indexes as data changes.

## Conclusion

For a carsharing application that will handle many concurrent users and transactions, the performance benefits of indexes far outweigh the costs. The selected indexes target the most common query patterns in the application, ensuring optimal performance for user experience.