import React, { useState, useEffect } from 'react';
import { Grid, IconButton, Typography, Box, CircularProgress } from '@mui/material';
import { Add, Remove, Delete } from '@mui/icons-material';
import { useCart } from './CartContext';
import { useAuth } from '../../contexts/AuthContext';
import authFetch from '../../AuthFetch';
import { jwtDecode } from 'jwt-decode';

const CartItem = ({ item }) => {
    const { removeFromCart, updateQuantity } = useCart();
    const { token, saveToken } = useAuth();
    const [userId, setUserId] = useState(null);
    const [book, setBook] = useState(null);
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        if (token)
            setUserId(jwtDecode(token).user_id);
    }, [token]);

    const handleIncrease = () => {
        authFetch(`http://127.0.0.1:8000/cart/${item.book_id}/increase`, {
            method: 'PUT',
        }, token, saveToken)
            .then((response) => response.json())
            .then((item) => {
                updateQuantity(item.book_id, item.quantity);
            })
            .catch((error) => {
                alert(error);
            });
    };

    const handleDecrease = () => {
        if (item.quantity > 1) {
            authFetch(`http://127.0.0.1:8000/cart/${item.book_id}/decrease`, {
                method: 'PUT',
            }, token, saveToken)
                .then((response) => response.json())
                .then((item) => {
                    updateQuantity(item.book_id, item.quantity);
                })
                .catch((error) => {
                    alert(error);
                });
        }
    };

    const handleRemove = () => {
        if (!userId)
            return;
        authFetch(`http://127.0.0.1:8000/cart/${item.book_id}/remove`, {
            method: 'DELETE',
        }, token, saveToken)
            .then((response) => response.json())
            .then((item) => {
                removeFromCart(item.book_id);
            })
            .catch((error) => {
                alert(error);
            });
    };

    useEffect(() => {
        setLoading(true);
        fetch(`http://127.0.0.1:8000/getBook/${item.book_id}`)
            .then((response) => response.json())
            .then((book) => {
                setBook(book);
            }).finally(() => {
                setLoading(false);
            });
    }, [item.book_id]);

    return (
        loading ? (<CircularProgress />) : (
            <Grid container spacing={2} alignItems="center" >
                <Grid item xs={6} sm={4}>
                    <Typography variant="body1">{book.name}</Typography>
                    <Typography variant="body2" color="textSecondary">{book.author}</Typography>
                </Grid>
                <Grid item xs={3}>
                    <Box display="flex" alignItems="center">
                        <IconButton onClick={handleDecrease} size="small">
                            <Remove />
                        </IconButton>
                        <Typography>{item.quantity}</Typography>
                        <IconButton onClick={handleIncrease} size="small">
                            <Add />
                        </IconButton>
                    </Box>
                </Grid>
                <Grid item xs={3}>
                    <Box display="flex" justifyContent="space-between" alignItems="center">
                        <Typography variant="body1">{(book.price * item.quantity).toFixed(2)} ₽</Typography>
                        <IconButton color="error" onClick={handleRemove}>
                            <Delete />
                        </IconButton>
                    </Box>
                </Grid>
            </Grid >)
    );
};

export default CartItem;