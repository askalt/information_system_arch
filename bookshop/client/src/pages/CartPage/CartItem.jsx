import React from 'react';
import { Grid, IconButton, Typography, Box } from '@mui/material';
import { Add, Remove, Delete } from '@mui/icons-material';
import { useCart } from './CartContext';

const CartItem = ({ item }) => {
    const { removeFromCart, updateQuantity } = useCart();

    const handleIncrease = () => updateQuantity(item.id, item.quantity + 1);
    const handleDecrease = () => {
        if (item.quantity > 1) {
            updateQuantity(item.id, item.quantity - 1);
        }
    };

    const handleRemove = () => removeFromCart(item.id);

    return (
        <Grid container spacing={2} alignItems="center">
            <Grid item xs={6} sm={4}>
                <Typography variant="body1">{item.name}</Typography>
                <Typography variant="body2" color="textSecondary">{item.author}</Typography>
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
                    <Typography variant="body1">{(item.price * item.quantity).toFixed(2)} ₽</Typography>
                    <IconButton color="error" onClick={handleRemove}>
                        <Delete />
                    </IconButton>
                </Box>
            </Grid>
        </Grid>
    );
};

export default CartItem;