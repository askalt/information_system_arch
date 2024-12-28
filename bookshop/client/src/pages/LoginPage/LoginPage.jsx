import React, { useState } from 'react';
import { Container, TextField, Button, Typography, Box } from '@mui/material';
import { useNavigate } from 'react-router-dom';
import { useAuth } from '../../contexts/AuthContext'

const LoginPage = () => {
    const [email, setEmail] = useState('');
    const [password, setPassword] = useState('');
    const [error, setError] = useState('');
    const { _, saveToken } = useAuth();

    const navigate = useNavigate();

    const handleSubmit = (e) => {
        e.preventDefault();

        fetch('http://127.0.0.1:8000/token', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: new URLSearchParams({
                email,
                password,
            }),
        })
            .then((response) => {
                if (!response.ok) {
                    setError('Неверный логин или пароль');
                    return;
                }
                return response.json();
            }).then((response) => {
                console.log(saveToken);
                saveToken(response.access_token);
                localStorage.setItem('refresh_token', response.refresh_token);
                navigate('/');
            });
    };

    return (
        <Container maxWidth="xs" sx={{ mt: 4 }}>
            <Typography variant="h5" gutterBottom>Авторизация</Typography>

            {error && (
                <Typography color="error" variant="body2" gutterBottom>{error}</Typography>
            )}

            <form onSubmit={handleSubmit}>
                <TextField
                    label="Email"
                    fullWidth
                    variant="outlined"
                    value={email}
                    onChange={(e) => setEmail(e.target.value)}
                    sx={{ mb: 2 }}
                />
                <TextField
                    label="Пароль"
                    type="password"
                    fullWidth
                    variant="outlined"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    sx={{ mb: 2 }}
                />
                <Button
                    type="submit"
                    variant="contained"
                    color="primary"
                    fullWidth
                >
                    Войти
                </Button>
            </form>

            <Box sx={{ mt: 2, textAlign: 'center' }}>
                <Typography variant="body2">
                    Нет аккаунта?{' '}
                    <Button color="secondary" onClick={() => navigate('/register')}>
                        Зарегистрироваться
                    </Button>
                </Typography>
            </Box>
        </Container>
    );
};

export default LoginPage;
