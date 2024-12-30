import React, { useState } from 'react';
import { Container, TextField, Button, Typography, Box } from '@mui/material';
import { useNavigate } from 'react-router-dom';

const RegisterPage = () => {
    const [name, setUsername] = useState('');
    const [email, setEmail] = useState('');
    const [password, setPassword] = useState('');
    const [confirmPassword, setConfirmPassword] = useState('');
    const [error, setError] = useState('');

    const navigate = useNavigate();

    const handleSubmit = (e) => {
        e.preventDefault();
        if (password !== confirmPassword) {
            setError('Пароли не совпадают');
            return;
        }
        if (email && password && name) {
            fetch('http://127.0.0.1:8000/register', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    email: email,
                    name: name,
                    password: password
                }),
            })
                .then((response) => {
                    if (!response.ok) {
                        console.error('Ошибка регистрации');
                        return;
                    }
                    navigate('/login');
                });
        } else {
            setError('Заполните все поля');
        }
    };

    return (
        <Container maxWidth="xs" sx={{ mt: 4 }}>
            <Typography variant="h5" gutterBottom>Регистрация</Typography>

            {error && (
                <Typography color="error" variant="body2" gutterBottom>{error}</Typography>
            )}

            <form onSubmit={handleSubmit}>
                <TextField
                    label="Имя пользователя"
                    fullWidth
                    variant="outlined"
                    value={name}
                    onChange={(e) => setUsername(e.target.value)}
                    sx={{ mb: 2 }}
                />
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
                <TextField
                    label="Подтверждение пароля"
                    type="password"
                    fullWidth
                    variant="outlined"
                    value={confirmPassword}
                    onChange={(e) => setConfirmPassword(e.target.value)}
                    sx={{ mb: 2 }}
                />
                <Button
                    type="submit"
                    variant="contained"
                    color="primary"
                    fullWidth
                >
                    Зарегистрироваться
                </Button>
            </form>

            <Box sx={{ mt: 2, textAlign: 'center' }}>
                <Typography variant="body2">
                    Уже есть аккаунт?{' '}
                    <Button color="secondary" onClick={() => navigate('/login')}>
                        Войти
                    </Button>
                </Typography>
            </Box>
        </Container>
    );
};

export default RegisterPage;
