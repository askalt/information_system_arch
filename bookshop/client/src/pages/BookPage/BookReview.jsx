import React, { useState, useEffect } from 'react';
import { Box, Avatar, Typography, Rating, IconButton } from '@mui/material';
import DeleteIcon from '@mui/icons-material/Delete';
import { jwtDecode } from 'jwt-decode';
import { useAuth } from '../../contexts/AuthContext'
//import { getUser } from '../../mocks/userService';

const Review = ({ review, onDelete }) => {
    const { token } = useAuth();
    const [user, setUser] = useState(null);
    const [userId, setUserId] = useState(null);

    useEffect(() => {
        if (token)
            setUserId(jwtDecode(token).user_id);
    }, [token]);

    useEffect(() => {
        fetch(`http://127.0.0.1:8000/getUser/${review.userId}`)
            .then((response) => response.json())
            .then((user) => {
                setUser(user);
            });
    }, [review.userId]);

    let user_image = user ? user.image : "";
    let user_name = user ? user.name : "";

    return (
        <Box sx={{ padding: 2, position: 'relative' }}>
            <Box sx={{ display: 'flex', alignItems: 'center', mb: 1 }}>
                <Avatar alt={user_name} src={user_image} sx={{ mr: 2, }} />
                <Typography variant="body1" fontWeight="bold">{user_name}</Typography>
            </Box>
            <Typography variant="body2" sx={{ textAlign: 'left' }}>{review.text}</Typography>

            <Box sx={{
                position: 'absolute',
                top: 10,
                right: 10,
                zIndex: 1,
            }}>
                <Rating value={review.rating} readOnly />
            </Box>
            {token && userId == review.userId && (
                <Box
                    sx={{
                        position: 'absolute',
                        bottom: 10,
                        right: 10,
                        zIndex: 2,
                    }}
                >
                    <IconButton
                        color="error"
                        onClick={() => onDelete(review.id)}
                        aria-label="delete"
                    >
                        <DeleteIcon />
                    </IconButton>
                </Box>
            )}
        </Box>
    );
};

export default Review;