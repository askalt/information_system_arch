import React, { createContext, useContext, useState } from 'react';

const CartContext = createContext();

export const useCart = () => {
    return useContext(CartContext);
};

export const CartProvider = ({ children }) => {
    const [cartItems, setCartItems] = useState([]);

    const addToCart = (book) => {
        setCartItems((prevItems) => {
            const existingItem = prevItems.find((item) => item.id === book.id);
            if (existingItem) {
                return prevItems.map((item) =>
                    item.id === book.id ? { ...item, quantity: item.quantity + 1 } : item
                );
            }
            return [...prevItems, { ...book, quantity: 1 }];
        });
    };

    const removeFromCart = (bookId) => {
        setCartItems((prevItems) => prevItems.filter((item) => item.id !== bookId));
    };

    const updateQuantity = (bookId, quantity) => {
        setCartItems((prevItems) =>
            prevItems.map((item) =>
                item.id === bookId ? { ...item, quantity } : item
            )
        );
    };

    return (
        <CartContext.Provider value={{ cartItems, addToCart, removeFromCart, updateQuantity }}>
            {children}
        </CartContext.Provider>
    );
};

export default CartProvider;