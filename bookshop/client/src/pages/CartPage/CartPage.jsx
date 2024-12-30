import { Box } from "@mui/material";
import React, { useState, useEffect } from 'react';
import { Grid, Typography, Divider, Button } from '@mui/material';
import CartItem from './CartItem';
import { useCart } from './CartContext';
import PriceTypography from '../../components/PriceTypography'
import { useAuth } from '../../contexts/AuthContext';
import authFetch from '../../AuthFetch';
import { jwtDecode } from 'jwt-decode';

const BasketPage = () => {
    const { cartItems, setCartItems } = useCart();
    const { token, saveToken } = useAuth();
    const [userId, setUserId] = useState(null);

    useEffect(() => {
        if (token)
            setUserId(jwtDecode(token).user_id);
    }, [token]);

    useEffect(() => {
        if (token) {
            authFetch(`http://127.0.0.1:8000/cart/${userId}/`, {}, token, saveToken)
                .then((response) => response.json())
                .then((items) => {
                    console.log(items);
                    setCartItems(items);
                })
                .catch((error) => {
                    alert(error);
                });
        }
    }, [userId]);

    //const total = cartItems.reduce((acc, item) => acc + item.price * item.quantity, 0);

    return (
        <Box p={2}>
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
            {token ? (
                <Box display="flex" justifyContent="space-between" alignItems="center">
                    {/*<PriceTypography variant="h6">Общая сумма: {total.toFixed(2)}₽</PriceTypography>*/}
                    <Button variant="contained" color="primary">Оформить заказ</Button>
                </Box>) : (<Box> Авторизуйтесь и добавляйте книги в корзину </Box>)}
        </Box>
    );
};

export default BasketPage;