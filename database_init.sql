-- Database initialization script for IUT Food System
-- This script creates the necessary tables for the restaurant menu management system

-- Users table (enhanced with restaurant_id)
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    user_type TEXT NOT NULL CHECK(user_type IN ('customer', 'manager', 'restaurant')),
    restaurant_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Restaurants table (enhanced with description)
CREATE TABLE IF NOT EXISTS restaurants (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    type TEXT NOT NULL,
    location TEXT NOT NULL,
    description TEXT,
    min_price INTEGER NOT NULL,
    max_price INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Menu items table for restaurant menus
CREATE TABLE IF NOT EXISTS menu_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    restaurant_id INTEGER,
    food_type TEXT NOT NULL,
    food_name TEXT NOT NULL,
    food_details TEXT NOT NULL,
    price INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Orders table for customer orders
CREATE TABLE IF NOT EXISTS orders (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    customer_id INTEGER,
    restaurant_id INTEGER,
    total_amount INTEGER NOT NULL,
    order_status TEXT DEFAULT 'pending',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Order items table for individual items in orders
CREATE TABLE IF NOT EXISTS order_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    order_id INTEGER,
    menu_item_id INTEGER,
    quantity INTEGER NOT NULL,
    price INTEGER NOT NULL,
    FOREIGN KEY (order_id) REFERENCES orders(id),
    FOREIGN KEY (menu_item_id) REFERENCES menu_items(id)
);

-- Insert sample restaurant data
INSERT OR IGNORE INTO restaurants (id, name, type, location, description, min_price, max_price) VALUES 
(1, 'Shaher', 'Fast Food', 'esfahanunivercity-sheikhbahaie-34', 'Delicious fast food restaurant serving burgers, sandwiches, and more', 500000, 600000),
(2, 'Aseman', 'Iranian', 'esfahanunivercity-sheikhbahaie-12', 'Authentic Iranian cuisine with traditional dishes and warm atmosphere', 10000, 100000);

-- Insert sample menu items
INSERT OR IGNORE INTO menu_items (restaurant_id, food_type, food_name, food_details, price) VALUES 
(1, 'Main Course', 'Classic Burger', 'Juicy beef burger with fresh vegetables and special sauce', 25000),
(1, 'Appetizer', 'French Fries', 'Crispy golden fries served with ketchup', 15000),
(1, 'Drink', 'Soft Drink', 'Refreshing carbonated beverage', 8000),
(2, 'Main Course', 'Kebab', 'Traditional Iranian kebab with rice and grilled vegetables', 30000),
(2, 'Soup', 'Ash Reshteh', 'Traditional Persian noodle soup with herbs and beans', 20000),
(2, 'Dessert', 'Sholeh Zard', 'Traditional Persian saffron rice pudding', 12000);

-- Insert sample users (you can modify these as needed)
INSERT OR IGNORE INTO users (username, password, user_type, restaurant_id) VALUES 
('restaurant1', 'password123', 'restaurant', 1),
('restaurant2', 'password123', 'restaurant', 2),
('customer1', 'password123', 'customer', NULL),
('manager1', 'password123', 'manager', NULL); 