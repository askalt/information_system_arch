import { Box } from "@mui/material";
import React from 'react';
import { Grid, Typography, Divider, Button } from '@mui/material';
import CartItem from './CartItem';
import { useCart } from './CartContext';

const BasketPage = () => {
    const { cartItems } = useCart();

    const total = cartItems.reduce((acc, item) => acc + item.price * item.quantity, 0);

    return (
        <Box p={2}>
            <Typography variant="h4" gutterBottom>Корзина</Typography>

            {cartItems.length === 0 ? (
                <Typography variant="h6" color="textSecondary">Ваша корзина пуста</Typography>
            ) : (
                <Grid container direction="column" spacing={2}>
                    {cartItems.map((item) => (
                        <Grid item key={item.id}>
                            <CartItem item={item} />
                        </Grid>
                    ))}
                </Grid>
            )}

            <Divider sx={{ my: 2 }} />

            <Box display="flex" justifyContent="space-between" alignItems="center">
                <Typography variant="h6">Общая сумма: {total.toFixed(2)} ₽</Typography>
                <Button variant="contained" color="primary">Оформить заказ</Button>
            </Box>
        </Box>
    );
};

export default BasketPage;